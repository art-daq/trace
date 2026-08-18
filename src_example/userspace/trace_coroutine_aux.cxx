// Second translation unit — built as a shared library.
#include "trace_coroutine.hxx"

#if __cplusplus >= 202002L

void run_template_version()
{
    TLOG() << "run_template_version() (aux TU)";
    EvenNumberProducer<int> producer(1, 10);
    auto evens = producer.generate();
    while (evens.move_next()) {
        TLOG() << "aux TU got value: " << evens.current_value();
    }
}

#endif
