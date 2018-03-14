import os
import sys
import subprocess

ATTEMPTS = int(sys.argv[1]) if len(sys.argv) > 1 else 10 
errors = 0

for _ in range(ATTEMPTS):
    s = subprocess.Popen(['./src/disastrOS_semaphore_test'])
    out, err = s.communicate()
    errors += 1 if s.returncode != 0 else 0

print('=====================' + '=' * ( len(str(errors)) + len(str(ATTEMPTS)) ))
print('Failure rate: {}/{} times'.format(errors, ATTEMPTS))
print('=====================' + '=' * ( len(str(errors)) + len(str(ATTEMPTS)) ))