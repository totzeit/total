total
=====

Simple command-line totaler. It adds numbers.

```
Usage total [OPTIONS] [FILE]
Where OPTIONS includes:

-s | --separator <sep>            Assume input fields separated by <sep>
-S | --output-separator <sep>     Output separated by <sep>
-f | --field <num>[,<num>]        Total field(s) <num>[,<num>]
-t | --totals-only                Only print grant totals
-z | --horizontal                 Total lines horizontally
-d | --doubles                    Assume values are floating point
-q | --quiet                      Ignore errors
-H | --header                     Print column headers
-h | --help                       You're soaking in it
-v | --version                    Probably idempotent

Defaults are:

  -f 1- -s ","

with the output separator the same as the input separator.

If FILE is left off, stdin is read. Note that in the presence of -z,
options -t and -H are ignored.
```

***

<a rel="license" href="http://creativecommons.org/licenses/by/3.0/"><img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by/3.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by/3.0/">Creative Commons Attribution 3.0 Unported License</a>.

***

Donations welcome
* BTC `13KGfWfYZ6VFd71sYHYRWFWSS1MBZDNcaW`
* LTC `LgUfbkJEdyeMS8Y2rBBfBQYKnDmqD2rKhm`
