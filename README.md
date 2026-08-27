### What is single_file?
A repository of single file scripts and snippets of code on arbitrary topics. I started this last year when I was still a student, and have been incrementally adding to it over the months. A lot of these scripts were written while I was reading some article, learning about some algorithm, doing a leetcode, or in transit like a plane or train with no drive to do anything else. I do like to revisit these from time to time though.

The scripts in the repository span the following languages: C++, Java, Python, Rust, and C. The vast majority of the scripts have been in C++. There’s no hard rule about the language I do it in though. It’s just whatever. You know the vibes.

Below is how to run/compile some of the languages used here.

```C++
// C++ (GCC Compiler)
g++ <file_name> -o <executable_name>
```

```C
// C (GCC Compiler)
gcc <file_name> -o <executable_name>
```

```Rust
// Rust
rustc <file_name>
```

```Python
# Python
python <file_name>
```

### Why am I sharing?
Truthfully, most of these are written on a whim. With that said, I think it showcases just how many things you can sit down and write some code about.

### Data Structure Scripts
Scripts like **red_black_tree_c**, **lp_hash_table_rs**, **string_interval_rs**, and **arena_cpp** explore data structures: a red black tree, an “open” linear-probe based hash table in rust, a large string “pool” that tracks each string’s interval/range, and a bump arena allocator in C++ that’s compatible with the standard library. There’s also **add_tree_java** which was a [joke file](https://x.com/tsoding/status/1996406860740706734?s=20), but can also be considered a data structure for operation trees. 

Due to the impromptu nature of most of these files, they’ll have a lot of fluff or extra stuff beyond the initial intent of the file. For example, arena_cpp also contains a simple Binary Search Tree written implementation that takes in an allocator, along with a templated Binary Tree type trait struct `BinaryTreeTraits`, and an iterator struct `InOrderTraversal` that works on any type that defines `BinaryTreeTraits` and supports the passing in of a scratch allocator. If you go through the files and think, “why is this here”, 9/10 times it’s me fucking around.

### Numerical Method Scripts
Scripts like **lagrange_py** and **simplex_cpp** are devoted to numerical methods, and were written while reading through articles on math.libretexts.org. 

The former script implements Lagrange Polynomial Interpolation as a simple class in Python, taking a list of discrete x and y points and fitting them to a continuous model/curve that can be sampled from. The libretexts article used for reference is [3.2 Polynomial Interpolation](https://math.libretexts.org/Courses/Angelo_State_University/Mathematical_Computing_with_Python/3%3A_Interpolation_and_Curve_Fitting/3.2%3A_Polynomial_Interpolation). 

The latter on the other hand is a rough implementation of the simplex algorithm for linear programming, which in of itself is an approach to solving linear optimization problems where you have a certain number of constraints and seek to maximize or minimize a particular value. The libretexts series used for reference is [7 Linear Programming](https://math.libretexts.org/Courses/SUNY_Geneseo/Math_113%3A_Finite_Math_for_Society/07%3A_Linear_Programming). In the code I only actually implement the maximization variant, so… while I hardly doubt anyone will actually use this, if you try to solve a minimization problem with it and it doesn’t work, I apologize in advance.

### Language Feature Scripts
The scripts dedicated to learning language features tend to be in C++, for obvious reasons. The language has way too many features. It’s very cool to learn about though. Some scripts in this area that I would like to highlight are **expression_template_cpp**, **iterator_adapters_cpp**, and **mixin_cpp**.

All three of these scripts revolve around a technique in C++ called the "[Curiously Recursive Template Pattern](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern),” which is when a child class is derived from a parent class that is templated on the child class itself ( e.g. `class Child : public Parent<Child> { … }` ). As it turns out, this parent-child class relationship lets you do a bunch of funky things.

The script expression_template_cpp focuses on “[Expression Templates](https://en.wikipedia.org/wiki/Expression_templates),” which are a technique for representing operation/expression trees and other structures of computation at compile-time. In the script I focus on implementing a simple 3d vector type `Vec3D` along with all its basic associated operations (addition, dot product, vector projection, normalizing, etc.) using Expression Templates and CRTP. By chaining together operations you can get extremely verbose type names like `ScaleVec3D<SumVec3D<SumVec3D<Vec3D,Vec3D>,Vec3D>>`,  for example. No matter how crazy the chain of operations may get, it’s all a form of lazy evaluation at the end of the day, and so it can all be evaluated back into a `Vec3D` again, in part due to CRTP.

Next we have iterator_adapters_cpp. Iterator adapters are iterators that take in an existing iterator and add behavior on top. If you’ve used python you’ve likely seen this in the form of map, zip, enumerate, filter, and so on. This is already a feature in C++ STL as of C++20 and onwards in the form of the `std::ranges` and `std::views` namespaces. What I wanted to mimic was the syntax of iterator adapters in Rust where you can repeatedly chain functions of the “Iterator trait” as if they were method calls (e.g. `iter.map(...).zip(...).enumerate()` ). Once again with the help of CRTP, it can be done in C++!

Lastly we have mixins_cpp. A “[Mixin](https://en.wikipedia.org/wiki/Mixin)” is a class that has access to methods of other classes with having to be the parent class of those other classes. In C++ this is made possible through multiple inheritance, variadic templates, and CRTP. Long story short, we implement a type `Mixin<A,B,C,...>` that has access to the methods of `A`, `B`, `C`, and so on.

### Graphics Scripts
The scripts **forward_kinematics_cpp** and **noise_c** are both computer graphics related. Forward Kinematics is typically used in computer graphics and robotics to calculate the position of an arm/bone or a point relative to an arm given the rotation of a joint. 

In forward_kinematics_cpp I implement basic 3d vector and 3-by-3 matrix types `vec3` & `mat3`, along with a simple example of configuring an arm’s basis & transformation matrices such that rotations are properly propogated along the arm. You can see a 2d desmos visualization of the implementation [here](https://www.desmos.com/calculator/cyabcuzgo1). Mess around with the *hw*, *h1*, and *h2* sliders and watch the "arm" spin around!

In noise_c we have an implementation of Value Noise in C. Value noise is a randomly generated signal that is designed to be smooth and continuous. If you’ve ever played minecraft, they use a similar kind of noise called ‘perlin noise’ to randomly generate terrain. This script supports various command line flags: a random seed, initial number of values, octaves of noise to apply, the period of the fundamental frequency/signal, the resolution of the signal (which scales both the number of values and the period), the amplitude of the signal, vertical translation of the signal, and interpolation method to apply during value noise (“linear” or “cubic-hermit interpolation/smoothstep”). Once again I have a 2d desmos visualization to accompany this script, which can be found [here](https://www.desmos.com/calculator/vdauxeo2qc).

### Operating System Features
For lack of a better name, I’m choosing to call the scripts in this section “Operating System Features.” Scripts like **virt_cpp** and **threads_c** play with some OS feature or try to mimic one. 

virt_cpp is an attempt at mimicking the paging system (in a single file of course). RAM and Disk memory are represented as simple arrays of characters. The kernel and individual processes are all represented with minimal structs. We have virtual page numbers (or more accurately, virtual page indices), page loading, page storing, page eviction, a bootleg free list of pages, and a bootleg translation look aside buffer for every process!

threads_c is a simple thread safe counter using locks. There’s not much else to it! There are some command line arguments that can be set. Flag -n <number> sets the count, and flag -t <number> sets the number of threads in play. Finally, the flag -a lets you set whether you’d like to use the normal mutex/lock based approach, or if you want to use the atomic_exchange_weak version instead. Regardless the outcome will be the same. The threads will work together to count all the way up to the target number, never exceeding it. A final printed message shows the individual contributions of each thread to the final counter. Maybe one thread did all the work, or maybe the distribution was uniform. Regardless, it's perfect for finding the slacker in the group.

### Misc.
A few scripts don’t fit anywhere. **pkmn_type_chart_rs**, and **igenous_rock_cpp** (yes, there’s a typo in the file name) are examples of this. 

pkmn_type_chart_rs is a pokemon type chart simulator that represents the type chart as a message passing graph. An input type is passed in, values are broadcast from node to node in the graph, and out comes the weaknesses a pokemon with those input type(s) would have. 

igenous_rock_cpp on the other hand is pretty lame. I wrote it in the morning during a Geology lab in university. You pass in a percentage of silica content a rock might have, and out comes a result telling you if the rock is felsic, intermediate, mafic, or ultramafic. The final rock type is determined by taking the mean of the silica content range each rock can have, and returning the nearest neighbor to the input. 
