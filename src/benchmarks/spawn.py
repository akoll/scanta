from s2bench.cpp2bench import Benchmark

benchmark = Benchmark(
  dir='spawn/',
  title='spawn',
  xlabel='entity count',
  ylabel='frame time',
  ylabel_right='memory consumption',
  axis_params='change y base, y SI prefix=milli, y unit=s,',
  axis_params_right='change y base, y SI prefix=mega, y unit=B,',
  main='../spawn.cpp',
  frames=1500,
  instrument='native',
  compile_params='-DRUNTIME_SEQUENTIAL -DINITIAL_COUNT=0 -DSPAWN_RATE=256',
  runs=[
    {
      'name': 'tuple of vectors',
      'compile_params': '-DBENCHMARK_MEMORY -DSTORAGE_TOV',
      'tex_params': '"orange!50,thick,dashed,const plot"',
      'side': 'right',
    },
    {
      'name': 'scattered',
      'compile_params': '-DBENCHMARK_MEMORY -DSTORAGE_SCATTERED',
      'tex_params': '"blue!50,thick,dashed,const plot"',
      'side': 'right',
    },
    {
      'name': 'tuple of vectors',
      'compile_params': '-DBENCHMARK_FRAMETIME -DSTORAGE_TOV',
      'tex_params': '"orange,const plot"',
      'instrument': 'frameavg',
      'repetitions': 24,
    },
    {
      'name': 'scattered',
      'compile_params': '-DBENCHMARK_FRAMETIME -DSTORAGE_SCATTERED',
      'tex_params': '"blue,const plot"',
      'instrument': 'frameavg',
      'repetitions': 24,
    },
  ]
)
