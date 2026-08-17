#include "trace_coroutine.hxx"

#if __cplusplus >= 202002L

// Defined in trace_coroutine_aux.cxx (shared library)
extern void run_template_version();

// ---------------------------------------------------------------------------
// NON-TEMPLATE (concrete) version for comparison.
// ---------------------------------------------------------------------------
Generator<int> generate_even_numbers(int start, int end) {
    TLOG() << "concrete coroutine starting";
    int current = (start % 2 == 0) ? start : start + 1;
    for (; current <= end; current += 2) {
        TLOG() << "concrete yielding: " << current;
        co_yield current;
    }
    TLOG() << "concrete coroutine ending";
}

// ---------------------------------------------------------------------------
int
main()
{
    TLOG() << "=== main() starting ===";

    // Template class member coroutine (main TU)
    TLOG() << "Template version (main TU):";
    EvenNumberProducer<int> producer(1, 10);
    auto evens_tmpl = producer.generate();
    while (evens_tmpl.move_next()) {
        TLOG() << "template got: " << evens_tmpl.current_value();
    }

    // Same template from the shared library
    TLOG() << "Template version (aux TU / shared library):";
    run_template_version();

    // Concrete non-template version
    TLOG() << "Concrete version:";
    auto evens = generate_even_numbers(1, 10);
    while (evens.move_next()) {
        TLOG() << "concrete got: " << evens.current_value();
    }

    TLOG() << "=== main() done ===";
    return 0;
}

#else

int
main()
{
    std::cerr << "trace_coroutine: this program was built with a compiler/standard"
                 " that does not support coroutines (need C++20 or later)." << std::endl;
    return 1;
}

#endif
