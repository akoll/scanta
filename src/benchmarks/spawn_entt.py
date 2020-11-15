from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun

runs = [
  Run(
    name='scatteredm',
    compile_params='-DBENCHMARK_MEMORY -DSTORAGE_SCATTERED'
  ),
  Run(
    name='tovm',
    compile_params='-DBENCHMARK_MEMORY -DSTORAGE_TOV'
  ),
  Run(
    name='enttm',
    compile_params='-DBENCHMARK_MEMORY -DSTORAGE_ENTT',
  ),
  Run(
    name='scatteredft',
    compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_SCATTERED',
    instrument='frameavg',
    repetitions=24,
  ),
  Run(
    name='tovft',
    compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_TOV',
    instrument='frameavg',
    repetitions=24,
  ),
  Run(
    name='enttft',
    compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_ENTT',
    instrument='frameavg',
    repetitions=24,
  ),
]

benchmark = Benchmark(
  dir='spawn_entt/',
  #title='spawn - sequential, $2500$ frames, spawn rate: $256$',
  title='',
  xlabel='entity count',
  ylabel='frame time',
  ylabel_right='memory consumption',
  axis_params='y unit=s,ymin=0,xmin=0,xmax=2499*256,ymode=log,log basis y=2,',
  axis_params_right='y unit=B,xmin=0,xmax=2499*256,ymin=0,ymode=log,log basis y=2,legend image code/.code={\\node[draw, rectangle, inner sep=1mm, fill] {#1};}',
  arrowheads=False,
  main='../spawn.cpp',
  frames=2500,
  compile_params='-DRUNTIME_SEQUENTIAL -DINITIAL_COUNT=0 -DSPAWN_RATE=256',
  legend_shift='0.05',
  runs=runs,
  plots=[
    Plot('scatteredm', title='scattered', tex_params='"draw=none,fill=blue!50,opacity=0.15,const plot" "x*256" y " \\\\closedcycle"', side='right', plotruns=[PlotRun(runs[0])]),
    Plot('enttm', title='EnTT', tex_params='"draw=green!50,pattern=north west lines,pattern color=green!50,opacity=0.15,const plot" "x*256" y " \\\\closedcycle"', side='right', plotruns=[PlotRun(runs[2])]),
    Plot('tovm', title='tuple of vectors', tex_params='"draw=orange,pattern=north east lines,pattern color=orange,opacity=0.15,const plot" "x*256" y " \\\\closedcycle"', side='right', plotruns=[PlotRun(runs[1])]),
    Plot('scatteredft', title='scattered', tex_params='"thick,blue,const plot" "x*256"', plotruns=[PlotRun(runs[3])]),
    Plot('enttft', title='EnTT', tex_params='"thick,green,const plot" "x*256"', plotruns=[PlotRun(runs[5])]),
    Plot('tovft', title='tuple of vectors', tex_params='"orange,const plot" "x*256"', plotruns=[PlotRun(runs[4])]),
  ]
)

benchmark.generate()
