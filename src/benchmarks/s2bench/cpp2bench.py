import os

class Step:
  def __init__(
    self,
    compile_params='',
    params='',
    repetitions=1,
    avg=False,
    max=False
  ):
    self.compile_params = compile_params
    self.params = params
    self.repetitions = repetitions
    self.avg = avg
    self.max = max

class PlotRun:
  def __init__(self, run, avg=False, max=False, min=False):
    self.run = run
    self.avg = avg
    self.max = max
    self.min = min

class Plot:
  def __init__(
    self,
    name,
    plotruns,
    title=None,
    side='left',
    tex_params='black',
    avg=False,
    max=False,
    min=False,
  ):
    self.name = name
    self.plotruns = plotruns
    if title == None:
      self.title = name
    else:
      self.title = title
    self.side = side
    self.tex_params = tex_params
    self.avg = avg
    self.max = max
    self.min = min

  def generate_post_processing(self, plotrun=None):
    return '{}{}{}'.format(
      ' | ../s2bench/bench2avg.py' if (plotrun.avg if plotrun != None else self.avg) else '',
      ' | ../s2bench/bench2max.py' if (plotrun.max if plotrun != None else self.max) else '',
      ' | ../s2bench/bench2min.py' if (plotrun.min if plotrun != None else self.min) else '',
    )

  def generate_commands(self):
    return '; '.join(['cat ' + plotrun.run.name.replace(' ', '_') + '.bench' + self.generate_post_processing(plotrun) for plotrun in self.plotruns])

class Run:
  def __init__(
    self,
    name,
    compile_params='',
    run_params='',
    steps=None,
    instrument='native'
  ):
    self.name = name
    self.compile_params = compile_params
    self.run_params = run_params
    self.instrument = instrument

    if steps == None:
      self.steps = [
        Step()
      ]
    else:
      self.steps = steps

  def is_homogenous(self):
    return len([step for step in self.steps if step.compile_params != self.steps[0].compile_params]) == 0

class Benchmark:
  def __init__(
    self,
    title,
    xlabel,
    ylabel,
    main,
    frames,
    runs=[],
    plots=[],
    compile_params='',
    run_params='',
    tex_params='',
    dir='build/',
    ylabel_right='',
    axis_params='',
    axis_params_right='',
    arrowheads=True
  ):
    self.title = title
    self.xlabel = xlabel
    self.ylabel = ylabel
    self.main = main
    self.frames = frames
    self.runs = runs
    self.plots = plots
    self.compile_params = compile_params
    self.run_params = run_params
    self.tex_params = tex_params
    self.dir = dir
    self.ylabel_right = ylabel_right
    self.axis_params = axis_params
    self.axis_params_right = axis_params_right
    self.arrowheads = arrowheads

  def generate(self):
    if not os.path.exists(self.dir):
      os.makedirs(self.dir)
    self.__graph()
    self.__makefile()

  def __generate_graph_includes(self, side):
    def include(plot):
      filename = '{}.tex'.format(
        plot.name.replace(' ', '_')
      )
      return r"""
        \input{%file%}
        \addlegendentry{%title%}
      """.rstrip().replace('%title%', plot.title).replace('%file%', filename) + '\n'

    return '\n'.join([include(plot) for plot in self.plots if plot.side == side])


  def __graph(self):
    # print('graph:', title, xlabel, ylabel, main, frames, runs)
    print('graph {}: {}'.format(self.main, self.title))
    file = open(self.dir + 'graph.tex', 'w')
    file.seek(0)
    file.write(r"""
\documentclass{article}
\usepackage{graphics}
\usepackage{tikz}
\usepackage{pgfplots}
\usepgfplotslibrary{units}

\begin{document}

\beginpgfgraphicnamed{bench}
  \begin{tikzpicture}
    \pgfplotsset{set layers}
    \begin{axis}[
      title={\textbf{%title%}},
      width=12cm, height=8cm,
      axis lines%star%=left,
      grid=major,
      mark size=0.4mm,
      xlabel={%xlabel%}, ylabel={%ylabel%},
      legend style={at={(0,-0.125)},anchor=north west},%axis_params%
    ]
%plots%
    \end{axis}
      """.strip()
      .replace('%title%', self.title)
      .replace('%xlabel%', self.xlabel)
      .replace('%ylabel%', self.ylabel)
      .replace('%axis_params%', self.axis_params)
      .replace('%star%', '*' if not self.arrowheads else '')
      .replace('%plots%', self.__generate_graph_includes('left'))
    )

    if self.ylabel_right != '':
      file.write(r"""
      \begin{axis}[
        width=12cm, height=8cm,
        axis lines%star%=right,
        axis x line=none,
        %grid=major,
        mark size=0.4mm,
        ylabel={%ylabel%},
        legend style={at={(1,-0.125)},anchor=north east},%axis_params%
      ]
  %plots%
      \end{axis}
        """.strip()
        .replace('%ylabel%', self.ylabel_right)
        .replace('%axis_params%', self.axis_params_right)
        .replace('%star%', '*' if not self.arrowheads else '')
        .replace('%plots%', self.__generate_graph_includes('right'))
      )

    file.write(r"""
  \end{tikzpicture}
\endpgfgraphicnamed

\end{document}
      """.strip()
    )
    file.truncate()
    file.close()

  def __makefile(self):
    file = open(self.dir + 'Makefile', 'w')
    file.seek(0)
    file.write("""
CC = g++
CFLAGS = -Wall -std=c++2a -O3 -fopenmp
CINCLUDES = -I../.. -I../../../lib -I../../../lib/taskflow -I../../../lib/entt/src
CPARAMS = -DFRAME_COUNT=%frames% %compile_params%

default: bench.pdf

      """.strip()
      .replace('%frames%', str(self.frames))
      .replace('%compile_params%', self.compile_params)
      + '\n\n'
    )

    filenames = [run.name.replace(' ', '_') for run in self.runs]
    for run, filename in zip(self.runs, filenames):
      print('run:', run.name)

      for step_index in (range(len(run.steps)) if not run.is_homogenous() else range(1)):
        step_cparams = run.steps[step_index].compile_params
        file.write("""
%outfile%: %main% $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) $(CINCLUDES) $(CPARAMS) %compile_params%
          """.strip()
          .replace('%main%', self.main)
          .replace('%compile_params%', '{} {}'.format(run.compile_params, step_cparams))
          .replace('%outfile%', filename + ('_' + str(step_index) if not run.is_homogenous() else '') +  '.out')
          + '\n\n'
        )

      file.write("""
%benchfile%: %outfiles%
	%steps% > %benchfile%
        """.strip()
        .replace('%outfiles%', ' '.join(([filename + '_' + str(index) + '.out' for index in range(len(run.steps))] if not run.is_homogenous() else [filename + '.out'])))
        .replace('%benchfile%', filename + '.bench')
        .replace('%steps%', self._render_steps(run, filename))
        + '\n\n'
      )

    for plot in self.plots:
      print('plot:', plot.name)

      file.write("""
%texfile%: %benchfiles%
	(%output%)%postproc% | ../s2bench/bench2tex.py %tex_params% > %texfile%
        """.strip()
        .replace('%tex_params%', plot.tex_params)
        .replace('%texfile%', plot.name.replace(' ', '_') + '.tex')
        .replace('%benchfiles%', ' '.join([plotrun.run.name.replace(' ', '_') + '.bench' for plotrun in plot.plotruns]))
        .replace('%output%', plot.generate_commands())
        .replace('%postproc%', plot.generate_post_processing())
        + '\n\n'
      )

    file.write("""
-include *.d

bench.pdf: graph.tex %texs%
	lualatex --jobname=bench graph.tex

clean:
	rm -rf bench.pdf *.out *.d *.bench *.log *.aux %texs%

.PHONY: default run clean
      """.strip()
      .replace('%texs%', ' '.join([plot.name.replace(' ', '_') + '.tex' for plot in self.plots]))
    )
      
    file.truncate()
    file.close()

  def _render_steps(self, run, filename):
    def step_command(index):
      step = run.steps[index]
      command = '{} {} {}'.format(
        './' + filename + ('_' + str(index) if not run.is_homogenous() else '') + '.out',
        run.run_params,
        step.params,
      )
      if (run.instrument == 'perf'):
        command = '(perf stat -e L1-dcache-loads,L1-dcache-load-misses -x , -r {} {} 2>&1) | ../s2bench/perf2bench.py'.format(step.repetitions, command)
      elif (run.instrument == 'frameavg'):
        command = '(for _ in {1..%s}; do %s; done) | ../s2bench/bench2avg.py %s' % (step.repetitions, command, step.repetitions)
      return command

    commands = '; '.join(['({}{}{})'.format(
      step_command(index),
       ' | ../s2bench/bench2avg.py' if (run.steps[index].avg) else '',
       ' | ../s2bench/bench2max.py' if (run.steps[index].max) else '',
    ) for index in range(len(run.steps))])
    if (len(run.steps) > 1):
      return '	(' + commands + ')'
    else:
      return '	' + commands
