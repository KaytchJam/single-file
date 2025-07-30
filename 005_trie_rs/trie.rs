use std::collections::hash_map::{HashMap, Iter};
use std::str::Chars;

/** Basic Node for our Trie data structure */
#[derive(Debug)]
struct TrieNode {
	character: char,
	terminal: bool,
	children: HashMap<char, usize>
}

/** TrieNode functions */
impl TrieNode {
	/** Create a new `TrieNode`. This node is assumed to be a terminal node with 0 children. */
	pub fn new(c: char) -> TrieNode {
		return TrieNode {
			character: c,
			terminal: true,
			children: HashMap::new()
		};
	}

	pub fn add_child(&mut self, child: char, index: usize) -> Option<usize> {
		self.terminal = false;
		return self.children.insert(child, index);
	}

	/** Returns whether this TrieNode contains a certain child character */
	pub fn has_child(&self, child: &char) -> bool {
		return self.children.contains_key(child);
	}

	/** Returns whether this node is terminal or not */
	pub fn is_terminal(&self) -> bool {
		return self.terminal;
	}

	pub fn iter_children(&self) -> Iter<'_, char, usize> {
		return self.children.iter();
	}

        pub fn child_index(&self, child: &char) -> Option<usize> {
                return self.children.get(&child).and_then(|c| Some(*c));
        }

	// pub fn index_of(&self, child: char) -> Option<&usize> {
	// 	return self.children.get(&child);
	// }

	// pub fn index_of_mut(&mut self, child: char) -> Option<&mut usize> {
	// 	return self.children.get_mut(&child);
	// }
}

/** The actual trie */
#[derive(Debug)]
struct Trie {
	data: Vec<TrieNode> // where everything actually is
}

impl Trie {
	pub fn new() -> Trie {
		return Trie {
			data: vec![TrieNode::new('R')] // 'R' for Rhut
		};
	}

	fn get_node(&self, idx: usize) -> &TrieNode {
		return &self.data[idx];
	}

	fn get_node_mut(&mut self, idx: usize) -> &mut TrieNode {
		return &mut self.data[idx];
	}
        
        /** Traverses a Trie given a word until the terminal node is found. Returns the terminal
         * node index and a character iterator */
        fn find_terminal<'t, 'w>(&'t self, word: &'w str) -> (usize, Option<char>, Chars<'w>) {
                let mut vanguard_idx: usize = 0;
                let mut char_iter: Chars<'w> = word.chars();   
                
                while let Some(c) = char_iter.next() {
                        if !self.get_node(vanguard_idx).has_child(&c) {
                                return (vanguard_idx, Some(c), char_iter);
                        }

                        vanguard_idx = self.get_node(vanguard_idx).child_index(&c).unwrap();
                }

                return (vanguard_idx, None, char_iter);
        }

        /** Adds a child character `c` to an existing node at `node_idx` */
        fn append_child(&mut self, node_idx: usize, child: char) -> usize {
            let new_idx: usize = self.data.len();
            let _ = self.get_node_mut(node_idx).add_child(child.clone(), new_idx);
            self.data.push(TrieNode::new(child));
            return new_idx;
        }

        /** Adds a word into our Trie */
	pub fn add(&mut self, word: &str) {
		let (mut vanguard_idx, copt, mut char_iter) = self.find_terminal(word);

                if copt.is_some() {
                        vanguard_idx = self.append_child(vanguard_idx, copt.unwrap());
                }

		for c in char_iter {
		        vanguard_idx = self.append_child(vanguard_idx, c);	
                }
	}

        /** Returns the number of letters within our trie */
        pub fn letters(&self) -> usize {
                return self.data.len() - 1;
        }

}

fn main() {
	let mut t: Trie = Trie::new();
	t.add("hooray");
        t.add("hoorah");
        t.add("hoe");
        t.add("horrible");
        print!("{:?}\n", t);
}
