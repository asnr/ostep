Running xv6 on macOS
--------------------

If you have followed the instructions on build xv6 below, add xv6 tools directory to `$PATH`

```sh
export PATH=/usr/local/xv6/bin:$PATH
```

and then, after changing the working directory to the xv6 root,

```sh
make clean && make qemu
```


Building xv6 on macOS
---------------------

To build xv6 on macOS (without a VM, i.e. cross-compiling an xv6 ELF binary), follow [these instructions](https://doesnotscale.com/compiling-xv6-on-os-x-10-9) with *only* the adjustments outlined below. I wasted a day trying to following these instructions but using the updated toolchain outlined in the [xv6 tools page](https://pdos.csail.mit.edu/6.828/2016/tools.html). So don't get clever and don't try to use different source versions where they aren't needed.

The adjustments that need to be made:
* Some of the `tar` commands don't work for reasons unknown. Where they didn't, I just inflated them using the mac `Finder`.
* When compiling `binutils`, change the `CFLAGS=-Wno-error=deprecated-declarations` option to `CFLAGS=-Wno-error`. Without this, you'll see the following error:
  
  ```
  elf32-i386.c:326:15: error: array index 42 is past the end of the array (which contains 34 elements)

  [-Werror,-Warray-bounds]

  return &elf_howto_table[R_386_IRELATIVE];
  ```

* When compiling `gcc`, add the option `MAKEINFO=missing`:

  ```
  ../gcc-4.5.1/configure MAKEINFO=missing --target=i386-jos-elf ...
  ```

  Without this option, you'll get an error that looks like `cppopts.texi:777: @itemx must follow @item`
* When installing the QEMU, use the updated git repo `http://web.mit.edu/ccutler/www/qemu.git -b 6.828-2.3.0`.
* When cloning xv6 use the public git repo `git://github.com/mit-pdos/xv6-public.git`
