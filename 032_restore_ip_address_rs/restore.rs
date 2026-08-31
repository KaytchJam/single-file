use std::env;
use std::fmt;

/** Vec wrapper type for sole use as a Stack */
#[derive(Debug)]
struct Stack<T> {
    buffer: Vec<T>
}

impl<T> Stack<T> {
    fn new() -> Stack<T> {
        return Stack {
            buffer: Vec::new()
        };
    }
    
    fn with_capacity(capacity: usize) -> Stack<T> {
        return Stack {
            buffer: Vec::with_capacity(capacity)
        }
    }

    fn push(&mut self, item: T) {
        self.buffer.push(item);
    }
    
    fn len(&self) -> usize {
        return self.buffer.len();
    }

    fn empty(&self) -> bool {
        return self.len() == 0;
    }
    
    fn top<'owner>(&'owner self) -> &'owner T {
        return &self.buffer[self.len() - 1];
    }
    
    fn top_mut<'owner>(&'owner mut self) -> &'owner mut T {
        let length: usize = self.len();
        return &mut self.buffer[length - 1];
    }

    fn pop(&mut self) {
        self.buffer.pop();
    }
}

/** Barebones representation of an IP4 Address: "octets[0].octets[1].octets[2].octets[2]" */
struct Ip4Addr {
    octets: [i32; 4]
}

impl fmt::Display for Ip4Addr {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        return write!(f, "{}.{}.{}.{}", self.octets[0], self.octets[1], self.octets[2], self.octets[3]);
    }
}

#[derive(Debug, Clone,Copy)]
struct StackItem {
    octet_size: i32,
    string_idx: i32
}

/** Parses an IP octet integer from a string. Returns `None` if:
 1) `end > string.len()`
 2) The string has leading zeroes
 3) The parsed integer is greater than 255 
 
 The string is assumed to have numeric characters ONLY. */
fn parse_ip_octet(string: &str, start: usize, end: usize) -> Option<i32> {
     if end > string.len() { return None; }
    
    let mut n: i32 = 0;
    for (idx, byte) in string[start..end].bytes().enumerate() {
        if idx > 0 && n == 0 { return None; }
        let digit = (byte - 0x30) as i32;
        n = (n * 10) + digit;
    }
    
    if n > 255 { return None; }
    return Some(n);
}

/** parse_ip_octet that takes in `StackItem`. */
fn parse_ip_octet_from_stack_item(string: &str, stack_data: &StackItem) -> Option<i32> {
    return parse_ip_octet(
        string, 
        stack_data.string_idx as usize, 
        (stack_data.string_idx + stack_data.octet_size) as usize
    );
}

const MAX_WINDOW_SIZE: usize = 3; // MAX WINDOW SIZE FOR PARSING IP OCTETS
const MAX_IP4_OCTET: usize = 4; // MAX NUMBER OF OCTETS IN AN IP4 ADDRESS

/** Iterative impelementation of the leetcode problem, 'restore_ip_addresses' */
fn restore_ip_addresses(string: &str) -> Vec<String> {
    let mut solutions: Vec<String> = Vec::new();
    let mut stack: Stack<StackItem> = Stack::with_capacity(MAX_IP4_OCTET + 1);

    let mut ip: Ip4Addr = Ip4Addr { octets: [0, 0, 0, 0] };
    let mut addr_at: usize = 1;

    stack.push(StackItem{ octet_size: MAX_WINDOW_SIZE as i32, string_idx: 0 });
    while !stack.empty() {
        let stack_info: StackItem = *stack.top();
        // Base Case 1: We've covered the entire string and/or we've "filled" all octets.
        if stack_info.string_idx as usize >= string.len() || addr_at > MAX_IP4_OCTET {
            if stack_info.string_idx as usize >= string.len() && addr_at > MAX_IP4_OCTET {
                solutions.push(ip.to_string());
            }
            addr_at -= 1;
            stack.pop();
        } else {
        // Base Case 2: We've exhausted all octet sizes.
            if stack_info.octet_size <= 0 {
                addr_at -= 1;
                stack.pop();
        // Normal Case: Check whether a valid IP octet can be formed at our location
        // in the string at the current string window size.
            } else {
                stack.top_mut().octet_size -= 1;
                if let Some(octet) = parse_ip_octet_from_stack_item(string, &stack_info) {
                    ip.octets[addr_at - 1] = octet;
                    stack.push(StackItem{ 
                        octet_size: MAX_WINDOW_SIZE as i32, 
                        string_idx: stack_info.string_idx + stack_info.octet_size 
                    });
                    addr_at += 1;
                }
            }
        }
    }

    return solutions;
}

fn main() {
    let mut ip_string: String = String::from("0000");
    let mut ip_updated: bool = false;

    for argument in env::args() {
        if argument.chars().all(char::is_numeric) {
            ip_string = argument;
            ip_updated = true;
        }
    }

    if !ip_updated {
        println!("Default input = \"{}\" is being used. Pass in a string of digits with length 1-12 as a cmd line argument, EX: \"123123123123\".", ip_string);
    }

    let addresses: Vec<String> = restore_ip_addresses(&ip_string);
    println!("============");
    if addresses.len() == 0 {
        println!("No valid IP addresses could be formed from the passed in string \"{}\"", ip_string);
    } else {
        println!("IP Addresses Produced from string \"{}\":", ip_string);
        for (idx, address) in addresses.iter().enumerate() {
            println!("ADDRESS #{}: \"{}\"", idx + 1, address);
        }
    }
}