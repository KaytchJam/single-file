
mod future {
    pub trait ForecasterMut {
        fn fit_mut<'read>(&mut self, data: &'read [f32]);
        fn predict<'me>(&'me self, horizon: usize) -> Vec<f32>;
    }
    
    /** Trait for types that try to see into the future */
    pub trait Forecaster: ForecasterMut + Sized {
        fn fit<'read>(mut self, data: &'read [f32]) -> Self {
            self.fit_mut(data);
            return self;
        }
    }
    
    pub struct GlobalTrend {
        slope: f32,
        y_intercept: f32
    }
    
    impl GlobalTrend {
        pub fn new() -> GlobalTrend {
            return GlobalTrend {
                slope: 0f32,
                y_intercept: 0f32
            };
        }
    }
    
    impl ForecasterMut for GlobalTrend {
        /** Fit the forecast model to the data */
        fn fit_mut(&mut self, data: &[f32]) {
            let n: usize = data.len();
            
            if n <= 0 {
                self.slope = 0.0;
                self.y_intercept = 0.0;
                return;
            }
            
            unsafe {
                let first = *data.get_unchecked(0); 
                let last = *data.get_unchecked(n - 1);
                self.slope = (last - first) / ((n - 1) as f32);
                self.y_intercept = last;
            }
        }
        
        /** Predict `horizon` future values */
        fn predict(&self, horizon: usize) -> Vec<f32> {
            let mut out: Vec<f32> = Vec::<f32>::with_capacity(horizon);
            for x in 0..horizon {
                out.push((x + 1) as f32 * self.slope + self.y_intercept);
            }
            return out;
        }
    }
    
    pub struct HoltLinearTrend {
        alpha: f32,
        beta: f32,
        last_level: f32,
        last_slope: f32
    }
    
    impl HoltLinearTrend {
        pub fn new() -> HoltLinearTrend {
            return HoltLinearTrend {
                alpha: 0.5,
                beta: 0.5,
                last_level: 0.0,
                last_slope: 0.0
            };
        }
    }
    
    impl ForecasterMut for HoltLinearTrend {
        fn fit_mut(&mut self, data: &[f32]) {
            for y in data {
                let cur_level = self.alpha * y + (1.0 - self.alpha) * (self.last_level + self.last_slope);
                self.last_slope = self.beta * (cur_level - self.last_level) + (1.0 - self.beta) * self.last_slope;
                self.last_level = cur_level;
            }
        }
        
        fn predict(&self, horizon: usize) -> Vec<f32> {
            let mut out = Vec::<f32>::with_capacity(horizon);
            for h in 0..horizon {
                let prediction: f32 = self.last_level + (h + 1) as f32 * self.last_slope;
                out.push(prediction);
            }
            return out;
        }
    }
    
    pub struct Naive {
        last_obs: f32
    }
    
    impl Naive {
        pub fn new() -> Naive {
            return Naive { last_obs: 0.0 };
        }
    }
    
    impl ForecasterMut for Naive {
        fn fit_mut(&mut self, data: &[f32]) {
            let n: usize = data.len();
            if n > 0 {
                unsafe { self.last_obs = *data.get_unchecked(n - 1); }
            }
        }
        
        fn predict(&self, horizon: usize) -> Vec<f32> {
            return vec![self.last_obs; horizon];
        }
    }
}

mod ml {
    pub fn train_test_split(data: &[f32], split: f32) -> (&[f32], &[f32]) {
        let n: usize = data.len();
        let cutoff: usize = ((n as f32) * split) as usize;
        return (&data[0..cutoff], &data[cutoff..n]);
    }
    
    pub fn calculate_sse(prediction: &[f32], truth: &[f32]) -> f32 {
        let n: usize = usize::min(prediction.len(), truth.len());
        let mut sse_total: f32 = 0.0;
        
        for idx in 0..n {
            let diff: f32 = truth[idx] - prediction[idx];
            sse_total += diff * diff;
        }
        
        return sse_total;
    }
}

mod utils {
    pub fn count_digits(mut i: i32) -> usize {
        if i == 0 { return 1; };

        let mut digits: usize = 0;
        while i != 0 {
            digits += 1;
            i /= 10;
        }

        return digits;
    }

    fn split_f32(f: f32) -> (i32, f32) {
        let v: f32 = f32::trunc(f);
        return (v as i32, f - v);
    }

    pub fn render_float(value: f32, places: usize) -> String {
        // first, take care of integer component & decimal component
        let (mut int_component, mut dec_component) = split_f32(value);

        let int_digits: usize = count_digits(int_component);
        let sign_offset: usize = if f32::is_sign_negative(value) { 1 } else { 0 };
        let total_size: usize = int_digits + sign_offset + places + 1; // extra 1 for the '.'

        let mut buffer: Vec<u8> = vec![b'0'; total_size];
        let mut buffer_idx: usize = 0;

        // populate buffer with integer side, and add the negative sign if necessary
        if sign_offset == 1 { 
            buffer[0] = b'-'; 
            buffer_idx += 1; 
        }
        
        if int_component == 0 {
            buffer[buffer_idx] = b'0';
            buffer_idx += 1;
        }

        int_component = i32::abs(int_component);
        let mut int_place: usize = 0;
        while int_component != 0 {
            let digit: i32 = int_component % 10;
            
            buffer[sign_offset + int_digits - int_place - 1] = (digit as u8) + b'0';
            int_place += 1;
            buffer_idx += 1;
            int_component /= 10;
        }

        // populate buffer with decimal side
        buffer[buffer_idx] = b'.';
        buffer_idx += 1;
        let places_start_idx: usize = buffer_idx;

        dec_component = f32::abs(dec_component);
        while (buffer_idx - places_start_idx < places) {
            dec_component *= 10.0;
            let digit: i32 = dec_component as i32;
            
            dec_component -= digit as f32;
            buffer[buffer_idx] = (digit as u8) + b'0';
            buffer_idx += 1;
        }

        return String::from_utf8(buffer).unwrap();
    }
}

fn main() {
    // Signal
    let data: Vec<f32> = vec![0f32, 1f32, 2f32, 3f32, 4f32, 5f32, 6f32, 7f32, 8f32, 9f32];
    let (train, test) = ml::train_test_split(data.as_slice(), 0.6);
    
    println!("Train: {:?}", train);
    println!("Test: {:?}", test);
    println!("\n=================\n");
    
    let mut forecasters: Vec<Box<dyn future::ForecasterMut>> = vec![
        Box::new(future::GlobalTrend::new()), 
        Box::new(future::HoltLinearTrend::new()),
        Box::new(future::Naive::new())
    ];

    let names: Vec<&str> = vec!["GlobalTrend", "HoltLinearTrend", "Naive"];
    for (monado, name) in forecasters.iter_mut().zip(names) {
        monado.fit_mut(train);

        let prediction: Vec<f32> = monado.predict(test.len());
        let error: f32 = ml::calculate_sse(prediction.as_slice(), test);
        println!("Forecast '{}': {:?}, Error = {}", name, prediction, utils::render_float(error, 5));
    } 
}