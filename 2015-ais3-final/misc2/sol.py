import sys
from collections import deque

sys.setrecursionlimit(385 * 385 + 100)
maze = [ list(l.strip()) for l in open('out.dat', 'r').readlines() ]
flag = [ [0] * 385 for _ in range(385) ]


A = (0, 383)
B = (384, 1)
D = (-1, 1)

def r_dfs(x, y, d=0):
    global A, B, maze, flag

    if (x, y) == B:
        return [B]

    if not (0 <= x < 385 and 0 <= y < 385):
        return False
    if flag[x][y] or maze[x][y] != ' ':
        return False

    flag[x][y] = 1

    for i in D:
        ret = r_dfs(x + i, y, d + 1)
        if ret:
            maze[x][y] = '.'
            return [ (x, y) ] + ret
        ret = r_dfs(x, y + i, d + 1)
        if ret:
            maze[x][y] = '.'
            return [ (x, y) ] + ret
    return False

    flag[x][y] = 0

def r(x, y):
    global A, B, maze, flag, path
    q = deque()
    q.append(A + (None, ))

    while len(q):
        p = q.popleft()
        x, y, link = p
        if (x, y) == B:
            return p

        if not (0 <= x < 385 and 0 <= y < 385):
            continue
        if flag[x][y] or maze[x][y] != ' ':
            continue
        flag[x][y] = 1
        for i in D:
            q.append((x + i, y, p))
            q.append((x, y + i, p))
    return False

# p = r_dfs(*A)
# print('\n'.join( 'r(%d, %d);' % pt for pt in p[::-1] ))

p = r(*A)
path = []
while p:
    x, y, link = p
    maze[x][y] = '.'
    path.append((x, y))
    p = link
open('path.txt', 'w').write('\n'.join( "r(%d, %d);" % p for p in path ))
open('result', 'w').write( '\n'.join( ''.join(line) for line in maze ) )
