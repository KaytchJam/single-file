use std::collections::{ HashMap, LinkedList };
use std::ptr;
use std::mem;

/** Representation of a weighted edge */
#[derive(Debug)]
struct Edge<W> {
    weight: W,
    target: usize
}

/** Representation of a message passing graph */
#[derive(Debug)]
struct SizedMessageGraph<const S: usize, W> {
    nodes: [W; S],   // treated as the "read-from" buffer
    message: [W; S], // treated as a "write-to" buffer
    edges: [LinkedList<Edge<W>>; S],
    acc: fn(W,W) -> W
}

impl <const S: usize, W: Default + Copy> SizedMessageGraph<S,W> {
    /** Constructs an empty SizedMessageGraph. All nodes start at W::default */
    pub fn new(node_initial: W, accumulation_func: fn(W,W) -> W) -> Self {
        return SizedMessageGraph {
            nodes: [node_initial; S],
            message: [node_initial; S],
            edges: std::array::from_fn(|_| LinkedList::new()),
            acc: accumulation_func
        };
    }

    /** Add an outgoing edge from node 'from' to node 'to' with some weight */
    pub fn add_edge(&mut self, from: usize, target: usize, weight: W) -> &mut Self {
        self.edges[from].push_back( Edge { weight, target } );
        return self;
    }

    /** Broadcast a message from node 'from' with weight 'weight'. This message
     * is then accumulated via the passed in accumulation function. */
    pub fn broadcast_from(&mut self, from: usize) -> &mut Self {
        let from_edges: &LinkedList<Edge<W>> = &self.edges[from];

        for edge_ref in from_edges.iter() {
            let target_weight: W = self.nodes[edge_ref.target];
            self.message[edge_ref.target] = (self.acc)(target_weight, edge_ref.weight);
        }

        return self;
    }

    /** Broadcast but it puts the message buffer results back into the nodes */
    pub fn broadcast_from_and_store(&mut self, from: usize) -> &mut Self {
        self.broadcast_from(from);
        mem::swap(&mut self.nodes, &mut self.message);
        return self;
    }

    /** Reset all node values to some default */
    pub fn reset(&mut self, node_initial: W) -> &mut Self {
        for i in 0..S {
            self.nodes[i] = node_initial;
            self.message[i] = node_initial;
        }
        return self;
    }

    pub fn message_buffer(&self) -> &[W;S] {
        return &self.message;
    }

    pub fn get_nodes(&self) -> &[W;S] {
        return &self.nodes;
    }
}

struct PokemonTypeGraph {
    resist_graph: SizedMessageGraph<18,f32>, // graph for determining what a type resists
    index2type: HashMap<&'static str, usize>,
    type_names: [&'static str; 18]
}

impl PokemonTypeGraph {

    /** 
     * Accumulation function to be used when determining
     * the degree of resistance of a certain type or
     * combination of types.
     */
    fn int_product(a: f32, b: f32) -> f32 {
        return a * b;
    }

    /** Initialize the PokemonTypeGraph */
    pub fn init() -> PokemonTypeGraph {
        let type_names = ["FIRE", "WATER", "GRASS", "ELECTRIC", "STEEL", "FIGHT", "GHOST", "ICE", "BUG",
            "PSYCHIC", "FLYING", "ROCK", "GROUND", "DRAGON", "FAIRY", "DARK", "NORMAL", "POISON"];

        // we don't do DE-SERIALIZE in the SINGLEFILE repository you clown. hard coded type shit
        let all_resistances = [
            vec![("GROUND", 2.0), ("ROCK", 2.0), ("BUG", 0.5), ("STEEL", 0.5), ("FIRE", 0.5), ("GRASS", 0.5), ("ICE", 0.5), ("FAIRY", 0.5)],
            vec![("STEEL", 0.5), ("FIRE", 0.5), ("WATER", 0.5), ("GRASS", 2.0), ("ELECTRIC", 2.0), ("ICE", 0.5)],
            vec![("FLYING", 2.0), ("POISON", 2.0), ("GROUND", 0.5), ("BUG", 2.0), ("FIRE", 2.0), ("WATER", 0.5), ("GRASS", 0.5), ("ELECTRIC", 0.5), ("ICE", 2.0)],
            vec![("FLYING", 0.5), ("GROUND", 2.0), ("STEEL", 0.5), ("ELECTRIC", 0.5)],
            vec![("NORMAL", 0.5), ("FIGHT", 2.0), ("FLYING", 0.5), ("POISON", 0.0), ("GROUND", 2.0), ("ROCK", 0.5), ("BUG", 0.5), ("STEEL", 0.5), ("FIRE", 2.0), ("GRASS", 0.5), ("PSYCHIC", 0.5), ("ICE", 0.5), ("DRAGON", 0.5), ("FAIRY", 0.5)],
            vec![("FLYING", 2.0), ("ROCK", 0.5), ("BUG", 0.5), ("PSYCHIC", 2.0), ("DARK", 0.5), ("FAIRY", 2.0)],
            vec![("NORMAL", 0.0), ("FIGHT", 0.0), ("POISON", 0.5), ("BUG", 0.5), ("GHOST", 2.0), ("DARK", 2.0)],
            vec![("FIGHT", 2.0), ("ROCK", 2.0), ("STEEL", 2.0), ("FIRE", 2.0), ("ICE", 0.5)],
            vec![("FIGHT", 0.5), ("FLYING", 2.0), ("GROUND", 0.5), ("ROCK", 2.0), ("FIRE", 2.0), ("GRASS", 0.5)],
            vec![("FIGHT", 0.5), ("BUG", 2.0), ("GHOST", 2.0), ("PSYCHIC", 0.5), ("DARK", 2.0)],
            vec![("FIGHT", 0.5), ("GROUND", 0.0), ("ROCK", 2.0), ("BUG", 0.5), ("GRASS", 0.5), ("ELECTRIC", 2.0), ("ICE", 2.0)],
            vec![("NORMAL", 0.5), ("FIGHT", 2.0), ("FLYING", 0.5), ("POISON", 0.5), ("GROUND", 2.0), ("STEEL", 2.0), ("FIRE", 0.5), ("WATER", 2.0), ("GRASS", 2.0)],
            vec![("POISON", 0.5), ("ROCK", 0.5), ("WATER", 2.0), ("GRASS", 2.0), ("ELECTRIC", 0.0), ("ICE", 2.0)],
            vec![("FIRE", 0.5), ("WATER", 0.5), ("GRASS", 0.5), ("ELECTRIC", 0.5), ("ICE", 2.0), ("DRAGON", 2.0), ("FAIRY", 2.0)],
            vec![("FIGHT", 0.5), ("POISON", 2.0), ("STEEL", 2.0), ("DRAGON", 0.0), ("DARK", 0.5)],
            vec![("FIGHT", 2.0), ("BUG", 2.0), ("GHOST", 0.5), ("PSYCHIC", 0.0), ("DARK", 0.5), ("FAIRY", 2.0)],
            vec![("FIGHT", 2.0), ("GHOST", 0.0)],
            vec![("FIGHT", 0.5), ("POISON", 0.5), ("GROUND", 2.0), ("BUG", 0.5), ("GRASS", 0.5), ("PSYCHIC", 2.0), ("FAIRY", 0.5)]
        ];

        let mut pkmn: PokemonTypeGraph = PokemonTypeGraph {
            resist_graph: SizedMessageGraph::new(1.0f32, Self::int_product),
            index2type: type_names.iter().enumerate().map(|e| (*e.1, e.0)).collect(),
            type_names
        };

        for (from_index, type_resistances) in all_resistances.iter().enumerate() {
            for (to_type, multiplier) in type_resistances {
                let to_index: usize = pkmn.type_to_index(to_type).unwrap();
                pkmn.resist_graph.add_edge(from_index, to_index, *multiplier);
            }
        }

        return pkmn;
    }

    /** Pass in an index and return its string mapping */
    pub fn index_to_type(&self, index: usize) -> Option<&'static str> {
        if index < self.type_names.len() {
            return Some(self.type_names[index]);
        }

        return None;
    }

    /** Pass in a pokemon type and return its index */
    pub fn type_to_index(&self, type_name: &str) -> Option<usize> {
        // convert to uppercase + other string cleaning
        return self.index2type.get(type_name).copied();
    }

    pub fn evaluate_weakness<const S: usize>(&mut self, typing: [&str; S]) -> [f32;18] {
        // convert to uppercase + other string cleaning

        // broadcast each type
        for type_name in typing {
            self.resist_graph.broadcast_from_and_store(self.type_to_index(type_name).unwrap());
        }

        let mut x = [0.0;18];
        x.copy_from_slice(self.resist_graph.get_nodes());
        self.resist_graph.reset(1.0);

        return x;
    }
}

fn main() {
    let mut pkmn: PokemonTypeGraph = PokemonTypeGraph::init();
    let sample_types = ["POISON", "PSYCHIC"];
    let weaknesses = pkmn.evaluate_weakness(sample_types);
    for (i, effectiveness) in weaknesses.iter().enumerate() {
        println!("{:?} <- ({} * {})", sample_types, pkmn.index_to_type(i).unwrap(), *effectiveness);
    }
}