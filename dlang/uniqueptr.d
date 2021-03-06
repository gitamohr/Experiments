import core.stdc.stdlib;
import std.functional;
import std.conv;

struct MallocFreePolicy
{
    pragma(inline, true) static auto alloc(T, Args...)(auto ref Args args) @nogc
    {
        auto ptr = cast(T*) malloc(T.sizeof);
        emplace(ptr, forward!args);
        return ptr;
    }

    pragma(inline, true) static void dealloc(T)(T* ptr) @nogc
    {
        destroy(*ptr);
        free(ptr);
    }
}

struct UniquePtr(T, AllocPolicy = MallocFreePolicy)
{
    T* _ptr;

    pragma(inline, true) this(U : T)(U* ptr) @nogc
    {
        _ptr = ptr;
    }

    this(this) @disable;

    pragma(inline, true) ~this() @nogc
    {
        if (_ptr != null)
        {
            AllocPolicy.dealloc(_ptr);
            _ptr = null;
        }
    }
}

struct Foo
{
    int _data;
}

void main()
{
    UniquePtr!Foo i = MallocFreePolicy.alloc!(Foo)(100);
}
