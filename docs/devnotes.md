
Setting up native rClr.so debugging from vs code.

`launch.json`

```json
        {
            // Adapted from debugging a native lib loaded by Python
            // https://code.visualstudio.com/docs/cpp/launch-json-reference
            // Not used as such but looking like a useful source: https://www.justinmklam.com/posts/2017/10/vscode-debugger-setup/
            // Maybe: https://marketplace.visualstudio.com/items?itemName=webfreak.debug
            // https://stackoverflow.com/questions/31763639/how-to-prevent-gdb-from-loading-debugging-symbol-for-a-large-library
            // I follow the instructions in https://developer.mozilla.org/en-US/docs/Archive/Mozilla/Using_gdb_on_wimpy_computers . May need adaptation to make it 
            // Nope. Need to use symbolLoadInfo below, but still stuck on libc6 exception
            // https://gist.github.com/asroy/ca018117e5dbbf53569b696a8c89204f
            "name": "(gdb) Attach to R session",
            "type": "cppdbg",
            "request": "attach",
            // "program": "/home/per202/src/csiro/stash/water-apportionment-pk/fortran/lib/libwaa.so",
            "program": "/usr/local/lib/R/bin/exec/R",
            "processId": "${command:pickProcess}",
            "MIMode": "gdb",
            "miDebuggerPath": "gdb",
            // "cwd": "/home/per202/src/csiro/stash/water-apportionment-pk/fortran/lib",
            "additionalSOLibSearchPath": "/home/per202/.local/lib/R/3.6.2/site-library/rClr/libs",
            "symbolLoadInfo":{
                "loadAll": false,
                "exceptionList": "rClr.so"
            },
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
```

```bash
export BUILDTYPE=Debug
R CMD INSTALL --no-test-load rClr
```
