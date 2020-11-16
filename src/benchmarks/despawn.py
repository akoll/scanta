from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun

runs = [
  Run(
    name='vecft',
    compile_params='',
    instrument='frameavg',
    repetitions=16,
  ),
  Run(
    name='setft',
    compile_params='-DSTORAGE_SCATTERED_SET',
    instrument='frameavg',
    repetitions=16,
  ),
]

benchmark = Benchmark(
  dir='despawn/',
  title='',
  xlabel='index',
  ylabel='time',
  #axis_params='y unit=s,ybar,bar width=1cm,ymin=0,xtick=data,xticklabels={set, {vector \\\\ from middle}, {vector \\\\ from front}, {vector \\\\ from back}},xmin=-0.5,xmax=3.5,xticklabel style={rotate=-45, anchor=west, xshift=-3mm,yshift=-2mm,align=center}',
  axis_params='change y base, y SI prefix=milli, y unit=s,ymin=0',
  main='../op_despawn.cpp',
  frames=1,
  compile_params='-DENTITY_COUNT=1000000 -DINTERVAL=3333',
  runs=runs,
  plots=[
    Plot('vecft', title='vector', tex_params='"thick,brown" "x*3333"', plotruns=[PlotRun(runs[0])]),
    Plot('setft', title='set', tex_params='"thick,blue" "x*3333"', plotruns=[PlotRun(runs[1])]),
  ]
)

benchmark.generate()
