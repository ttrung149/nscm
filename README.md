# nanoscheme

Maybe tinier, C++ version of [Norman Ramsey](https://www.cs.tufts.edu/~nr/)'s uscheme language.

## Installing

Clone the repository. Change directory to root folder and create the binary using

```sh
make nscm
```

## Docs

After creating the binary from source run `./nscm --help` to see the help menu. Run `./nscm` to start the REPL.

Primitives supported include the following

```
if, define, set                                          -- Control flow, var assign
+, -, *, /, mod, >, >=, <, <=, =                         -- Arithmetic operations
number?, symbol?, procedure?, list?, string?, boolean?   -- Type check
equal?, sin, cos, tan, sqrt, log, abs                    -- Math operations
lambda,                                                  -- Lambda expression
car, cdr, cons, null?, map, filter, append               -- List operations
```

## Examples

There are some basic .scm testing files in the `examples/` folder. Run the following to import the examples"

```sh
./nscm examples/test.scm examples/list.scm
```

## Authors

* [Trung Truong](https://github.com/ttrung149)

## Contributing

Fork the repo and submit a pull request. Check out [TODO](TODO) to see what hasn't been implemented.

## License

This project is licensed under the MIT License - see the [LICENSE.txt](LICENSE.txt) file for details