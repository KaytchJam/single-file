use std::hash::Hash;
use std::hash::Hasher;
use std::hash::DefaultHasher;

/** Super simple constant size Linear Probing Hash Table */

#[derive(Debug)]
struct OpenHashTable<const S: usize, K, V> {
    buffer: [Option<(K,V)>; S],
    capacity: usize
}

impl<const S: usize, K: Hash + Eq, V> OpenHashTable<S,K,V> {
    pub fn new() -> Self {
        return OpenHashTable {
            buffer: [ const { None } ; S],
            capacity: S
        };
    }

    pub fn size(&self) -> usize {
        return S;
    }

    fn find_hash(key: &K) -> usize {
        let mut hasher = DefaultHasher::new();
        key.hash(&mut hasher);
        return hasher.finish() as usize;
    }

    fn find_open_index(&self, mut index: usize) -> usize {
        while self.buffer[index].is_some() {
            index = (index + 1) % S;
        }

        return index;
    }

    pub fn is_empty(&self) -> bool {
        return self.capacity == 0;
    }

    pub fn insert(&mut self, key: K, value: V) {
        if self.is_empty() {
            return;
        }

        let index: usize = self.find_open_index(Self::find_hash(&key) % S);
        self.buffer[index] = Some((key, value));
        self.capacity -= 1;
    }

    pub fn get(&self, key: &K) -> Option<&V> {
        let mut index: usize = Self::find_hash(&key) % S;
        let mut count: usize = 0;
       
        // Loop till we find the equal item
        while count < S && (self.buffer[index].is_none() || self.buffer[index].as_ref().unwrap().0 != *key) {
            index = (index + 1) % S;
            count += 1;
        }

        if count == S || self.buffer[index].is_none() {
            return None;
        }

        return self.buffer[index].as_ref().map(|p| &p.1);
    }
}

fn main() {
    println!("Hello world");
    let mut oht: OpenHashTable<10,char,u32> = OpenHashTable::new();

    let mut index: u32 = 0;
    while !oht.is_empty() {
        oht.insert(char::from_u32(index + 65).expect("All u32s"), index);
        index += 1;
    }

    println!("{:?}", oht);
    println!("HashTable size: {}", oht.size());

    let c: char = 'C';
    println!("Get {} = ({},{})", c, c, oht.get(&c).unwrap());
}

