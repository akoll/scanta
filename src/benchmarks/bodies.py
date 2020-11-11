from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun

benchmark = Benchmark(
  dir='bodies/',
  main='../bodies.cpp',
  frames=1000,
  compile_params='-DRUNTIME_SEQUENTIAL -DSTORAGE_TOV -DWIDTH=128',
  runs=[
    Run(name='rigidft', compile_params='-DBENCHMARK_FRAMETIME -DOPTIMIZE_RIGID', run_params='100000', steps=[
      Step(compile_params='-DSKIP_SOFT', avg=True),
      Step(compile_params='-DSKIP_RIGID', avg=True),
    ]),
    Run(name='softft', compile_params='-DBENCHMARK_FRAMETIME -DOPTIMIZE_SOFT', run_params='100000', steps=[
      Step(compile_params='-DSKIP_SOFT', avg=True),
      Step(compile_params='-DSKIP_RIGID', avg=True),
    ]),
    Run(name='rigidcm', compile_params='-DOPTIMIZE_RIGID', run_params='100000', instrument='perf', repetitions=2, steps=[
      Step(compile_params='-DSKIP_SOFT'),
      Step(compile_params='-DSKIP_RIGID'),
    ]),
    Run(name='softcm', compile_params='-DOPTIMIZE_SOFT', run_params='100000', instrument='perf', repetitions=2, steps=[
      Step(compile_params='-DSKIP_SOFT'),
      Step(compile_params='-DSKIP_RIGID'),
    ]),
  ],
)

benchmark.generate()
