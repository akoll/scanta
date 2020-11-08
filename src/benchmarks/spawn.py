from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun

runs = [
  Run(
    name='tovm',
    compile_params='-DBENCHMARK_MEMORY -DSTORAGE_TOV'
  ),
  Run(
    name='scatteredm',
    compile_params='-DBENCHMARK_MEMORY -DSTORAGE_SCATTERED',
  ),
  Run(
    name='tovft',
    compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_TOV',
    instrument='frameavg',
    repetitions=24,
  ),
  Run(
    name='scatteredft',
    compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_SCATTERED',
    instrument='frameavg',
    repetitions=24,
  ),
]

benchmark = Benchmark(
  dir='spawn/',
  title='spawn - sequential, $2500$ frames, spawn rate: $256$',
  xlabel='entity count',
  ylabel='frame time',
  ylabel_right='memory consumption',
  axis_params='change y base, y SI prefix=milli, y unit=s,',
  axis_params_right='change y base, y SI prefix=mega, y unit=B,',
  main='../spawn.cpp',
  frames=2500,
  compile_params='-DRUNTIME_SEQUENTIAL -DINITIAL_COUNT=0 -DSPAWN_RATE=256',
  runs=runs,
  plots=[
    Plot('tovm', title='tuple of vectors', tex_params='"draw=none,fill=orange!50,opacity=0.5,ultra thick,const plot" x y " \\\\closedcyle"', side='right', legend_entry=False, plotruns=[PlotRun(runs[0])]),
    Plot('scatteredm', title='scattered', tex_params='"draw=none,fill=blue!50,opacity=0.5,ultra thick,const plot" x y " \\\\closedcyle"', side='right', legend_entry=False, plotruns=[PlotRun(runs[1])]),
    Plot('tovft', title='tuple of vectors', tex_params='"orange,const plot"', plotruns=[PlotRun(runs[2])]),
    Plot('scatteredft', title='scattered', tex_params='"blue,const plot"', plotruns=[PlotRun(runs[3])]),
  ]
)

benchmark.generate()
