#include "BenchmarkFixture.hpp"

class ProgramBenchmarks : public BenchmarkFixture
{};


BENCHMARK_F(ProgramBenchmarks, Mandelbrot128x128)(benchmark::State& st)
{
    // Calculates a Mandelbrot "image" with 128x128 pixels resolution, stored in megabuf.
    // This is the worst Mandelbrot implementation using lots of multiplications.
    // The inner loop code executes about 4.6 million times.
    auto code = projectm_eval_code_compile(m_context, R"(
        size_x = 128;
        size_y = 128;
        pos_x = 0;
        loop(size_x,
            pos_y = 0;
            loop(size_y,
                x0 = -2.00 + ((0.47 - -2.00) / size_x) * pos_x; // X range
                y0 = -1.12 + ((1.12 - -1.12) / size_y) * pos_y; // Y range
                x = 0;
                y = 0;
                iteration = 0;
                while(
                    xtemp = sqr(x) - sqr(y) + x0;
                    y = 2*x*y + y0;
                    x = xtemp;
                    iteration += 1;
                    sqr(x) + sqr(y) <= 4 && iteration < 1000
                );
                megabuf(posy * size_y + posx) = iteration;
                pos_y += 1
            );
            pos_x += 1
        );
    )");

    for (auto _ : st) {
        projectm_eval_code_execute(code);
    }
}
