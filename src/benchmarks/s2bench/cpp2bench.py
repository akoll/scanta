import os

class Benchmark:
  def __init__(self, title, xlabel, ylabel, main, frames, runs, instrument='native', compile_params='', run_params='', tex_params='', ymax=None, dir='build/', ylabel_right='', ymax_right=None, axis_params='', axis_params_right=''):
    self.dir = dir
    if not os.path.exists(dir):
      os.makedirs(dir)
    self.__graph(title, xlabel, ylabel, main, frames, runs, ymax, ylabel_right, ymax_right, axis_params, axis_params_right)
    self.__makefile(main, frames, compile_params, run_params, tex_params, runs, instrument)

  def __graph(self, title, xlabel, ylabel, main, frames, runs, ymax=None, ylabel_right='', ymax_right=None, axis_params='', axis_params_right=''):
    # print('graph:', title, xlabel, ylabel, main, frames, runs)
    print('graph {}: {}'.format(main, title))
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
      axis lines=left,
      grid=major,
      mark size=0.4mm,
      xlabel={%xlabel%}, ylabel={%ylabel%},%ymax%
      legend style={at={(0,-0.1)},anchor=north west},%axis_params%
    ]
%runs%
    \end{axis}
      """.strip()
      .replace('%title%', title)
      .replace('%xlabel%', xlabel)
      .replace('%ylabel%', ylabel)
      .replace('%ymax%', ' ymax={},'.format(ymax) if ymax else '')
      .replace('%axis_params%', axis_params)
      .replace('%runs%', '\n'.join([self.__plot(runs[index], index) for index in range(len(runs)) if 'side' not in runs[index] or runs[index]['side'] == 'left']))
    )

    if ylabel_right != '':
      file.write(r"""
      \begin{axis}[
        width=12cm, height=8cm,
        axis lines=right,
        axis x line=none,
        grid=major,
        mark size=0.4mm,
        ylabel={%ylabel%},%ymax%
        legend style={at={(1,-0.1)},anchor=north east},%axis_params%
      ]
  %runs%
      \end{axis}
        """.strip()
        .replace('%ylabel%', ylabel_right)
        .replace('%ymax%', ' ymax={},'.format(ymax_right) if ymax_right else '')
        .replace('%axis_params%', axis_params_right)
        .replace('%runs%', '\n'.join([self.__plot(runs[index], index) for index in range(len(runs)) if 'side' in runs[index] and runs[index]['side'] == 'right']))
      )

    file.write(r"""
  \end{tikzpicture}
\endpgfgraphicnamed

\end{document}
      """.strip()
    )
    file.truncate()
    file.close()

  def __plot(self, run, index):
    name = run['name']
    print('plot:', name)
    filename = str(index) + '_' + name.replace(' ', '_') + '.tex'
    return r"""
      \input{%file%}
      \addlegendentry{%name%}
    """.rstrip().replace('%name%', name).replace('%file%', filename) + '\n'

  def __makefile(self, main, frames, compile_params, run_params, tex_params, runs, instrument):
    file = open(self.dir + 'Makefile', 'w')
    file.seek(0)
    file.write("""
CC = g++
CFLAGS = -Wall -std=c++2a -O3 -fopenmp
CINCLUDES = -I../.. -I../../../lib -I../../../lib/taskflow -I../../../lib/entt/src
CPARAMS = -DFRAME_COUNT=%frames% %compile_params%

default: bench.pdf

      """.strip()
      .replace('%frames%', str(frames))
      .replace('%compile_params%', compile_params)
      + '\n\n'
    )

    filenames = [str(index) + '_' + runs[index]['name'].replace(' ', '_') for index in range(len(runs))]
    for run, filename in zip(runs, filenames):
      name = run['name']
      compile_params = run['compile_params'] if 'compile_params' in run else ''
      tex_params = run['tex_params'] if 'tex_params' in run else ''

      homogenous = 'steps' not in run or len([step for step in run['steps'] if 'compile_params' in step]) == 0
      run['homogenous'] = homogenous
      for step_index in (range(len(run['steps'])) if not homogenous else range(1)):
        step_cparams = run['steps'][step_index]['compile_params'] if 'steps' in run and 'compile_params' in run['steps'][step_index] else ''
        file.write("""
%outfile%: %main% $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) $(CINCLUDES) $(CPARAMS) %compile_params%
          """.strip()
          .replace('%main%', main)
          .replace('%compile_params%', '{} {}'.format(compile_params, step_cparams))
          .replace('%outfile%', filename + ('_' + str(step_index) if not homogenous else '') +  '.out')
          + '\n\n'
        )

      file.write("""
%texfile%: graph.tex %outfiles%
%steps% | ../s2bench/bench2tex.py %tex_params% > %texfile%
        """.strip()
        .replace('%tex_params%', tex_params)
        .replace('%outfiles%', ' '.join(([filename + '_' + str(index) + '.out' for index in range(len(run['steps']))] if not homogenous else [filename + '.out'])))
        .replace('%texfile%', filename + '.tex')
        .replace('%steps%', self._render_steps(run, instrument, filename))
        + '\n\n'
      )

    file.write("""
-include *.d

bench.pdf: graph.tex %texs%
	lualatex --jobname=bench graph.tex

clean:
	rm -rf bench.pdf *.out *.d *.log *.aux %texs%

.PHONY: default run clean
      """.strip()
      .replace('%texs%', ' '.join([file + '.tex' for file in filenames]))
    )
      
    file.truncate()
    file.close()

  def _render_steps(self, run, instrument, filename):
    if 'steps' not in run:
      run['steps'] = [{ 'params': run['run_params'] if 'run_params' in run else '' }]

    run_instrument = run['instrument'] if 'instrument' in run else instrument
    def step_command(index):
      step = run['steps'][index]
      repetitions = step['repetitions'] if 'repetitions' in step else run['repetitions'] if 'repetitions' in run else 1
      command = '{} {}'.format(
        './' + filename + ('_' + str(index) if not run['homogenous'] else '') + '.out',
        step['params'],
      )
      if (run_instrument == 'perf'):
        command = '(perf stat -e L1-dcache-loads,L1-dcache-load-misses -x , -r {} {} 2>&1) | ../s2bench/perf2bench.py'.format(repetitions, command)
      return command

    commands = '; '.join(['({}{})'.format(
      step_command(index),
      ' | ../s2bench/bench2avg.py' if ('average' in run['steps'][index] and run['steps'][index]['average']) or ('average' not in run['steps'][index] and 'averages' in run and run['averages']) else '',
    ) for index in range(len(run['steps']))])
    if (len(run['steps']) > 1):
      return '	(' + commands + ')'
    else:
      return '	' + commands
