
class Main {
    static interface AddLike {
        public int get();
    }
    
    static class Add implements AddLike {
        public Add(int a, int b) {
            this.a = a;
            this.b = b;
        }
        
        public int get() { // magic function
            return a + b;
        }
        
        private int a, b;
    }
    
    static class AddTree implements AddLike {
        public AddTree(Main.AddLike a, Main.AddLike b) {
            this.l = a;
            this.r = b;
        }
        
        public AddTree(int a, int b) {
            this.l = new Add(a, 0);
            this.r = new Add(b, 0);
        }
        
        public AddTree add(int a) {
            return new AddTree(this, new Add(a, 0));
        }
        
        public AddTree add(AddLike a) {
            return new AddTree(this, a);
        }
        
        public int get() { // sorcery
            return l.get() + r.get();
        }
        
        private Main.AddLike l, r;
    }
    
    public static void main(String[] args) {
        Main.AddTree a1 = new Main.AddTree(6, 7);
        Main.AddTree b1 = new Main.AddTree(7, 1);
        System.out.println(a1.add(b1).get());
    }
}