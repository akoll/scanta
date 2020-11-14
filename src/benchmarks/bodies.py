from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun, PlotStub

runs = [
  Run(name='rigidft', compile_params='-DBENCHMARK_FRAMETIME -DSKIP_SOFT', run_params='100000', steps=[
    Step(compile_params='-DOPTIMIZE_RIGID', avg=True),
    Step(compile_params='-DOPTIMIZE_SOFT', avg=True),
  ]),
  Run(name='softft', compile_params='-DBENCHMARK_FRAMETIME -DSKIP_RIGID', run_params='100000', steps=[
    Step(compile_params='-DOPTIMIZE_RIGID', avg=True),
    Step(compile_params='-DOPTIMIZE_SOFT', avg=True),
  ]),
  Run(name='rigidcm', compile_params='-DSKIP_SOFT', run_params='100000', instrument='perf', repetitions=2, steps=[
    Step(compile_params='-DOPTIMIZE_RIGID'),
    Step(compile_params='-DOPTIMIZE_SOFT'),
  ]),
  Run(name='softcm', compile_params='-DSKIP_RIGID', run_params='100000', instrument='perf', repetitions=2, steps=[
    Step(compile_params='-DOPTIMIZE_RIGID'),
    Step(compile_params='-DOPTIMIZE_SOFT'),
  ]),
]
  
benchmark = Benchmark(
  dir='bodies/',
  main='../bodies.cpp',
  frames=1000,
  compile_params='-DRUNTIME_SEQUENTIAL -DSTORAGE_TOV -DWIDTH=128',
  title='',
  width=10,
  arrowheads=False,
  ylabel='average frame time',
  axis_params='change y base, y SI prefix=milli, y unit=s,ymin=0,xmin=-0.5,xmax=1.5,ybar,xtick=data,xticklabels={{optimize for rigid bodies}, {optimize for soft bodies}},grid=both,minor tick num=3',
  ylabel_right='cache misses',
  axis_params_right='y unit=\\%,ymin=0,xmin=-0.5,xmax=1.5,ybar,',
  runs=runs,
  plots=[
    Plot('ssbft', title='skip soft bodies', tex_params='fill=brown', plotruns=[PlotRun(runs[0])]),
    Plot('srbft', title='skip rigid bodies', tex_params='fill=cyan', plotruns=[PlotRun(runs[1])]),
    PlotStub(),
    PlotStub(),
    PlotStub(side='right'),
    PlotStub(side='right'),
    Plot('ssbcm', title='skip soft bodies', side='right', tex_params='"pattern color=brown,pattern=crosshatch,"', plotruns=[PlotRun(runs[2])]),
    Plot('srbcm', title='skip rigid bodies', side='right', tex_params='"pattern color=cyan,pattern=crosshatch,"', plotruns=[PlotRun(runs[3])]),
  ]
)

benchmark.generate()
