use std::ops::{Range, Deref};
use std::str;
use std::collections::HashMap;
use std::borrow::Borrow;
use std::hash::{Hash, Hasher};

/** For storing multiple strings inside one shared buffer instead of
* each individual string being heap allocated. */
struct StringIntervals {
    strings: Vec<Range<u32>>, // indices of each string in our byte buffer
    bytes: Vec<u8> // contiguous buffer of bytes
}

/** A  */
struct StringIntervalIndex<'a> {
    index: usize,
    string_interval: &'a StringIntervals
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
    pub fn insert(&mut self, in_string: &str) -> usize {
        let start_byte = self.get_next_start_byte();
        let byte_interval = start_byte..(start_byte + in_string.bytes().len() as u32);
        
        let idx: usize = self.strings.len();
        self.strings.push(byte_interval);
        self.bytes.extend_from_slice(in_string.as_bytes());

        return idx;
    }
    
    /** Return a reference to a String Slice in the StringIntervals structure */
    pub fn get(&self, index: usize) -> &str {
        let range = &self.strings[index];
        return str::from_utf8(&self.bytes[(range.start as usize)..(range.end as usize)]).unwrap();
    }

    #[allow(dead_code)]
    pub fn get_mut(&mut self, index: usize) -> &mut str {
        let range = &self.strings[index];
        return str::from_utf8_mut(&mut self.bytes[(range.start as usize)..(range.end as usize)]).unwrap();
    }

    pub fn make_heavy_index<'a>(&'a self, index: usize) -> StringIntervalIndex<'a> {
        return StringIntervalIndex {
            index,
            string_interval: self
        };
    }
}

impl<'a> StringIntervalIndex<'a> {
    #[allow(dead_code)]
    fn new(string_interval: &'a StringIntervals, index: usize) -> StringIntervalIndex<'a> {
        return StringIntervalIndex {
            index,
            string_interval
        };
    }
}

impl<'a> Borrow<str> for StringIntervalIndex<'a> {
    fn borrow(&self) -> &'a str {
        return self.string_interval.get(self.index);
    }
}

impl<'a> Deref for StringIntervalIndex<'a> {
    type Target = str;

    fn deref(&self) -> &Self::Target {
        return self.string_interval.get(self.index);
    }
}

impl<'a> PartialEq for StringIntervalIndex<'a> {
    fn eq(&self, other: &Self) -> bool {
        return Borrow::<str>::borrow(self) == Borrow::<str>::borrow(other);
    }
}

impl<'a> Eq for StringIntervalIndex<'a> {}

impl<'a> Hash for StringIntervalIndex<'a> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        Borrow::<str>::borrow(self).hash(state);
    }
}

fn main() {
    let mut intervals = StringIntervals::new(10);
    
    let a = intervals.insert("Canticle");
    let b = intervals.insert("Aleys");
    let c = intervals.insert("Finn");
    let d = intervals.insert("Lukas");
    
    for i in 0..intervals.len() {
        println!("{}", intervals.get(i));
    }

    let ai = intervals.make_heavy_index(a);
    let bi = intervals.make_heavy_index(b);
    let ci = intervals.make_heavy_index(c);
    let di = intervals.make_heavy_index(d);

    let mut map: HashMap<StringIntervalIndex, i32> = HashMap::new();
    map.insert(ai, 10);
    map.insert(bi, 13);
    map.insert(ci, 25);
    map.insert(di, 45);

    let names = ["Canticle", "Aleys", "Finn"];
    for name in names {
        if let Some(count) = map.get(name) {
            println!("{} : {}", name, count);
        }
    }

    let last = String::from("Lukas");
    if let Some(last_count) = map.get(last.as_str()) {
        println!("{} : {}", last, last_count)
    }
}