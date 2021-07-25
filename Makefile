coro:
	clang main.c coro.c scheduler.c -fuse-ld=lld -o coro

coro-fs:
	clang main.c coro.c scheduler.c -fuse-ld=lld -nostdlib -static -DFREESTANDING -o coro
