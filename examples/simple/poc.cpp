/**
 * test file from mr-j0nes @github
 * Proof of concept Raftlib
 *
 * Want to have a 3 kernels stream which produces and sends 10 numbers down the stream.
 * The 10 numbers should be created by the first kernel and destroyed by the last.
 *
 */
#include <raft>
#include <raftio>

/**
 * Producer: sends down the stream numbers from 1 to 10
 */
class A : public raft::kernel
{
private:
    int i = 0;
    int cnt = 0;

public:
    A() : raft::kernel()
    {
        output.addPort<int>("out");
    }

    virtual raft::kstatus run()
    {
        i++;

        if (i <= 10) 
        {
            auto c(output["out"].template allocate_s<int>());
            *c = i;
            std::cout << "A: " << ++cnt << ":" << *c << " out_addr:" << static_cast<void*>(&(*c)) << std::endl;
            output["out"].send();
        }

        if (i > 20) return (raft::stop);


        return (raft::proceed);
    };
};

/**
 * Processor: It simmulates a process with the input numbers
 */
class B : public raft::kernel
{
private: 
    int cnt = 0;
public:
    B() : raft::kernel()
    {
        input.addPort<int>("in");
        output.addPort<int>("out");
    }

    virtual raft::kstatus run()
    {

        auto &input_port((this)->input["in"]);
        auto &a(input_port.template peek<int>());

        // FIXME How to pass the input memory to the output ?
        auto c(output["out"].template allocate_s<int>());
        (*c) = a;
        std::cout << "B: " << ++cnt << ":" << a << " in_addr:" << static_cast<void*>(&a);
        std::cout << " out_addr:" << static_cast<void*>(&(*c)) << std::endl;
        input_port.recycle(1);
        // input_port.unpeek(); // We don't really need to unpeek here do we ?
        return (raft::proceed);
    }
};

/**
 * Consumer: takes the number from input and dumps it to the console
 */
class C : public raft::kernel
{
private:
    int cnt = 0;
public:
    C() : raft::kernel()
    {
        input.addPort<int>("in");
    }

    virtual raft::kstatus run()
    {

        auto &input_port((this)->input["in"]);
        auto &a(input_port.template peek<int>());

        std::cout << "C: " << ++cnt << ":" << a << " in_addr:" << static_cast<void*>(&a) << std::endl;

        input_port.recycle(1);

        return (raft::proceed);
    }
};

int main()
{
    A a;
    B b;
    C c;

    raft::map m;
    
    m += a >> b >> c;

    m.exe();

    return( EXIT_SUCCESS );
}
