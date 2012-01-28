class B;

class A
{
    B& b_;
public:
   explicit A(B& b);

   B& get_b() { return b_; }
};
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
