use std::ops::Range;
use std::str;

/** For storing multiple strings inside one shared buffer instead of
* each individual string being heap allocated. */
struct StringIntervals {
    strings: Vec<Range<u32>>, // indices of each string in our byte buffer
    bytes: Vec<u8> // contiguous buffer of bytes
}

impl StringIntervals {
    /** Create a new string interval with some initial size */
    pub fn new(size: usize) -> StringIntervals {
        return StringIntervals {
            strings: Vec::with_capacity(size),
            bytes: Vec::with_capacity(size)
        };
    }
    
    /** Return the length of the string interval */
    pub fn len(&self) -> usize {
        return self.strings.len();
    }
    
    fn get_next_start_byte(&self) -> u32 {
        let num_strings = self.strings.len();
        if num_strings == 0 {
            return 0;
        }
        
        return self.strings[num_strings - 1].end;
    }
    
    /** Insert a string into the StringIntervals structure */
    pub fn insert(&mut self, in_string: &str) {
        let start_byte = self.get_next_start_byte();
        let byte_interval = start_byte..(start_byte + in_string.bytes().len() as u32);
        
        self.strings.push(byte_interval);
        self.bytes.extend_from_slice(in_string.as_bytes());
    }
    
    /** Return a reference to a String Slice in the StringIntervals structure */
    pub fn get_slice(&self, index: usize) -> &str {
        let range = &self.strings[index];
        return str::from_utf8(&self.bytes[(range.start as usize)..(range.end as usize)]).unwrap();
    }
}

fn main() {
    let mut intervals = StringIntervals::new(10);
    
    intervals.insert("Canticle");
    intervals.insert("Aleys");
    intervals.insert("Finn");
    intervals.insert("Lukas");
    
    for i in 0..intervals.len() {
        println!("{}", intervals.get_slice(i));
    }
}