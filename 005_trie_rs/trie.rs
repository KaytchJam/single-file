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
	pub fn new(c: char, t_state: bool) -> TrieNode {
		return TrieNode {
			character: c,
			terminal: t_state,
			children: HashMap::new()
		};
	}

	pub fn add_child(&mut self, child: char, index: usize) -> Option<usize> {
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

        pub fn set_terminal(&mut self, terminal_state: bool) -> &mut Self {
                self.terminal = terminal_state;
                return self;
        }

	pub fn iter_children(&self) -> Iter<'_, char, usize> {
		return self.children.iter();
	}

        pub fn child_index(&self, child: &char) -> Option<usize> {
                return self.children.get(&child).and_then(|c| Some(*c));
        }
}

/** The actual trie */
#[derive(Debug)]
struct Trie {
	data: Vec<TrieNode> // where everything actually is
}

/** Iterator for the Trie struct */
struct Words<'p> {
        parent: &'p Trie,
        stack: Vec<Iter<'p,char,usize>>,
        builder: String
}

impl Trie {
        /** Construct an empty Trie */
	pub fn new() -> Trie {
		return Trie {
			data: vec![TrieNode::new('R', false)] // 'R' for Rhut
		};
	}
        
        /** Might be useful in 10 years */
        fn get_root() -> usize {
                return 0;
        }

	fn get_node(&self, idx: usize) -> &TrieNode {
		return &self.data[idx];
	}

	fn get_node_mut(&mut self, idx: usize) -> &mut TrieNode {
		return &mut self.data[idx];
	}
        
        /** Traverses a Trie given a word until the terminal node is found. Returns the terminal
         * node index, the character before the iteration broke, and the used character iterator */
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
                self.data.push(TrieNode::new(child, false));
                return new_idx;
        }

        /** Adds a word into our Trie */
	pub fn add(&mut self, word: &str) {
		let (mut vanguard_idx, copt, mut char_iter) = self.find_terminal(word);
                match copt {
                        Some(c) => { vanguard_idx = self.append_child(vanguard_idx, c); },
                        None => { self.get_node_mut(vanguard_idx).set_terminal(true); }
                }
    
		for c in char_iter {
		        vanguard_idx = self.append_child(vanguard_idx, c);	
                }

                self.get_node_mut(vanguard_idx).set_terminal(true);
	}

        /** Returns the number of letters within our trie */
        pub fn letters(&self) -> usize {
                return self.data.len() - 1;
        }

        /** Does our trie contain the word */
        pub fn contains(&self, word: &str) -> bool {
                let mut cur_node: usize = 0;
                for c in word.chars() {
                        let child_idx: Option<usize> = self.get_node(cur_node).child_index(&c);
                        if child_idx.is_none() {
                                return false;
                        }

                        cur_node = child_idx.unwrap();
                }

                return self.get_node(cur_node).is_terminal();
        }

        pub fn words<'s>(&'s self) -> Words<'s> {
                return Words::new(self);
        }
}

impl<'p> Words<'p> {
        fn new<'d>(parent: &'d Trie) -> Words<'d> {
                return Words {
                        parent,
                        stack: vec![parent.get_node(0).iter_children()],
                        builder: String::with_capacity(10)
                };
        }

        fn top(&mut self) -> &mut std::collections::hash_map::Iter<'p,char,usize> {
                let top_idx: usize = self.stack.len() - 1;
                return &mut self.stack[top_idx];
        }

        fn push(&mut self, node_idx: usize) {
                let iter = self.parent.get_node(node_idx).iter_children();
                self.stack.push(iter);
        }

        fn is_terminal(&self, node_idx: usize) -> bool {
                return self.parent.get_node(node_idx).is_terminal();
        }

        fn next_word(&mut self) -> Option<String> {
                while self.stack.len() > 0 {
                        let front: &mut std::collections::hash_map::Iter<'p,char,usize> = self.top();
                        let child = front.next();

                        match child {
                                Some(c) => { 
                                        self.builder.push(*c.0);
                                        self.push(*c.1);
                                        if self.is_terminal(*c.1) {
                                                return Some(self.builder.as_str().to_string());
                                        }
                                },

                                None => {
                                        self.builder.pop();
                                        self.stack.pop();
                                }
                        }
                }

                return None;
        }
}

impl<'p> Iterator for Words<'p> {
        type Item = String;
    // our item is a string slice of the current String held by the iterator
    // if the user wants to store the string elsewhere they need to call to_owned() on it or smthn

        fn next(&mut self) -> Option<Self::Item> {
        // how do we iterate? recursive exploration of child nodes
        // need to append previous chars as we descend. pop off as we ascend back
        // return word formed as string when we hit a "terminal" node
                return self.next_word();
        }
}

fn main() {
	let mut t: Trie = Trie::new();
	t.add("hooray");
        t.add("hoorah");
        t.add("hoe");
        t.add("horrible");
        t.add("hoo");

        let target: &'static str = "hoo";
        println!("Our trie {} contain the word {}.",
            if t.contains(target) { "does" } else { "does not" },
            target
        );

        for word in t.words() {
                println!("{}", word);
        }
}
