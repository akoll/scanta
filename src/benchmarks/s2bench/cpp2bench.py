import os

def filenamify(name):
  return name.replace(' ', '_').replace('(', '+').replace(')', '+')

class Step:
  def __init__(
    self,
    compile_params='',
    params='',
    repetitions=None,
    avg=False,
    max=False,
    min=False,
    slope=False,
  ):
    self.compile_params = compile_params
    self.params = params
    self.repetitions = repetitions
    self.avg = avg
    self.min = min
    self.max = max
    self.slope = slope

class PlotRun:
  def __init__(self, run, cbench=False, avg=False, max=False, min=False, slope=False):
    self.run = run
    self.cbench = cbench
    self.avg = avg
    self.max = max
    self.min = min
    self.slope = slope

  def get_run_name(self):
    if self.cbench:
      return self.run.name + '.compile'
    else:
      return self.run.name

class PlotStub:
  def __init__(self, side='left'):
    self.side = side

class Plot:
  def __init__(
    self,
    name,
    plotruns,
    title=None,
    side='left',
    tex_params='black',
    legend_entry=True,
    avg=False,
    max=False,
    min=False,
    slope=False,
  ):
    self.name = name
    self.plotruns = plotruns
    if title == None:
      self.title = name
    else:
      self.title = title
    self.side = side
    self.tex_params = tex_params
    self.legend_entry = legend_entry
    self.avg = avg
    self.max = max
    self.min = min
    self.slope = slope

  def generate_post_processing(self, plotrun=None):
    return '{}{}{}{}'.format(
      ' | ../s2bench/bench2avg.py' if (plotrun.avg if plotrun != None else self.avg) else '',
      ' | ../s2bench/bench2max.py' if (plotrun.max if plotrun != None else self.max) else '',
      ' | ../s2bench/bench2min.py' if (plotrun.min if plotrun != None else self.min) else '',
      ' | ../s2bench/bench2slope.py' if (plotrun.slope if plotrun != None else self.slope) else '',
    )

  def generate_commands(self):
    return '; '.join(['cat ' + filenamify(plotrun.get_run_name()) + '.bench' + self.generate_post_processing(plotrun) for plotrun in self.plotruns])

class Run:
  def __init__(
    self,
    name,
    compile_params='',
    run_params='',
    steps=None,
    instrument='native',
    repetitions=1,
  ):
    self.name = name
    self.compile_params = compile_params
    self.run_params = run_params
    self.instrument = instrument
    self.repetitions = repetitions

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
    main,
    frames,
    title=None,
    width=12,
    height=8,
    xlabel='',
    ylabel='',
    runs=[],
    plots=[],
    compile_params='',
    run_params='',
    tex_params='',
    dir='build/',
    ylabel_right='',
    axis_params='',
    axis_params_right='',
    arrowheads=True,
    legend_shift=0
  ):
    self.title = title
    self.width = width
    self.height = height
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
    self.legend_shift = legend_shift

  def generate(self):
    if not os.path.exists(self.dir):
      os.makedirs(self.dir)
    if self.title != None:
      self.__graph()
    self.__makefile()

  def __generate_graph_includes(self, side):
    def include(plot):
      include = ""
      if isinstance(plot, Plot):
        filename = '{}.tex'.format(
          filenamify(plot.name)
        )
        include = r"""
        \input{%file%}
        """.rstrip().replace('%file%', filename) + '\n'
      elif isinstance(plot, PlotStub):
        include = r"""
        \addplot coordinates {};
        """
      else:
        print('ERROR: plot is neither Plot nor PlotStub')
        exit(1)
      if isinstance(plot, Plot) and plot.legend_entry:
        include += r"""
          \addlegendentry{%title%}
        """.rstrip().replace('%title%', plot.title) + '\n'
      return include

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
\usetikzlibrary{patterns}
\usepackage{pgfplots}
\pgfplotsset{compat=1.17}
\usepgfplotslibrary{units}

\pgfrealjobname{full}
\begin{document}

\beginpgfgraphicnamed{bench}
  \begin{tikzpicture}
    \pgfplotsset{set layers}
    """)

    if self.ylabel_right != '':
      file.write(r"""
      \begin{axis}[
        width=%width%cm, height=%height%cm,
        axis y line%star%=right,
        %hide x axis,
        axis x line=none,
        %grid=major,
        mark size=0.4mm,
        ylabel={%ylabel%},
        legend style={at={(1,-0.125-%shift%)},anchor=north east},%axis_params%
      ]
  %plots%
      \end{axis}
        """.strip()
        .replace('%ylabel%', self.ylabel_right)
        .replace('%width%', str(self.width))
        .replace('%height%', str(self.height))
        .replace('%axis_params%', self.axis_params_right)
        .replace('%star%', '*' if not self.arrowheads else '')
        .replace('%shift%', self.legend_shift)
        .replace('%plots%', self.__generate_graph_includes('right'))
      )

    file.write(r"""
    \begin{axis}[
      title={\textbf{%title%}},
      width=%width%cm, height=%height%cm,
      axis lines%star%=left,
      grid=major,
      mark size=0.4mm,
      xlabel={%xlabel%}, ylabel={%ylabel%},
      legend style={at={(0,-0.125-%shift%)},anchor=north west},%axis_params%
    ]
%plots%
    \end{axis}
      """.strip()
      .replace('%title%', self.title)
      .replace('%width%', str(self.width))
      .replace('%height%', str(self.height))
      .replace('%xlabel%', self.xlabel)
      .replace('%ylabel%', self.ylabel)
      .replace('%axis_params%', self.axis_params)
      .replace('%star%', '*' if not self.arrowheads else '')
      .replace('%shift%', self.legend_shift)
      .replace('%plots%', self.__generate_graph_includes('left'))
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
      """.strip()
      .replace('%frames%', str(self.frames))
      .replace('%compile_params%', self.compile_params)
      + '\n\n'
    )

    if self.title != None:
      file.write('default: bench.pdf\n\n')
    else:
      file.write('default: {}\n'.format(' '.join([filenamify(run.name) + '.bench' for run in self.runs])))

    filenames = [filenamify(run.name) for run in self.runs]
    for run, filename in zip(self.runs, filenames):
      print('run:', run.name)

      for step_index in (range(len(run.steps)) if not run.is_homogenous() else range(1)):
        step_cparams = run.steps[step_index].compile_params
        file.write("""
%outfile%: %main% $(DEPS)
	\\time -f %e $(CC) -MMD -o $@ $< $(CFLAGS) $(CINCLUDES) $(CPARAMS) %compile_params% 2>%concat% %runname%.compile.bench
          """.strip()
          .replace('%main%', self.main)
          .replace('%compile_params%', '{} {}'.format(run.compile_params, step_cparams))
          .replace('%outfile%', filename + ('_' + str(step_index) if not run.is_homogenous() else '') +  '.out')
          .replace('%runname%', filename)
          .replace('%concat%', '>' if step_index != 0 else '')
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
      if isinstance(plot, Plot):
        print('plot:', plot.name)

        file.write("""
  %texfile%: %benchfiles%
	(%output%)%postproc% | ../s2bench/bench2tex.py %tex_params% > %texfile%
          """.strip()
          .replace('%tex_params%', plot.tex_params)
          .replace('%texfile%', filenamify(plot.name) + '.tex')
          .replace('%benchfiles%', ' '.join([filenamify(plotrun.get_run_name()) + '.bench' for plotrun in plot.plotruns]))
          .replace('%output%', plot.generate_commands())
          .replace('%postproc%', plot.generate_post_processing())
          + '\n\n'
        )

    file.write('-include *.d\n\n')

    if self.title != None:
      file.write("""
bench.pdf: graph.tex %texs%
	lualatex --jobname=bench graph.tex
        """.strip()
        .replace('%texs%', ' '.join([filenamify(plot.name) + '.tex' for plot in self.plots if isinstance(plot, Plot)]))
        + '\n\n'
      )

    file.write("""
clean:
	rm -rf *.pdf *.out *.d *.bench *.log *.aux %texs%

.PHONY: default run clean
      """.strip()
      .replace('%texs%', ' '.join([filenamify(plot.name) + '.tex' for plot in self.plots if isinstance(plot, Plot)]))
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
      repetitions = step.repetitions if step.repetitions != None else run.repetitions
      if (run.instrument == 'perf'):
        command = '(perf stat -e L1-dcache-loads,L1-dcache-load-misses -x , -r {} {} 2>&1) | ../s2bench/perf2bench.py'.format(repetitions, command)
      elif (run.instrument == 'frameavg'):
        command = '(for _ in {1..%s}; do %s; done) | ../s2bench/bench2avg.py %s' % (repetitions, command, repetitions)
      return command

    commands = '; '.join(['({}{}{}{}{})'.format(
      step_command(index),
       ' | ../s2bench/bench2avg.py' if (run.steps[index].avg) else '',
       ' | ../s2bench/bench2max.py' if (run.steps[index].max) else '',
       ' | ../s2bench/bench2min.py' if (run.steps[index].min) else '',
       ' | ../s2bench/bench2slope.py' if (run.steps[index].slope) else '',
    ) for index in range(len(run.steps))])
    if (len(run.steps) > 1):
      return '	(' + commands + ')'
    else:
      return '	' + commands
