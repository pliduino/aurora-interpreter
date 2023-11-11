import fnmatch
import os

exec_name = "exec"
debug = ARGUMENTS.get('debug', False)

env = Environment(CPATH = {"src/"})

if bool(debug):
    env.Append(CFLAGS = ['/Z7', '/W4'])
    env.Append(LINKFLAGS = ['/DEBUG'])

src = []
for root, dirnames, filenames in os.walk('src'):
  for filename in fnmatch.filter(filenames, '*.c'):
    src.append(os.path.join(root, filename));

t = env.Program(f"out/{exec_name}", src)
Clean(t, f'out/{exec_name}.pdb')
Clean(t, f'out/{exec_name}.ilk')