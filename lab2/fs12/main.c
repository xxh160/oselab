#include <fs.h>
#include <engine.h>
#include <common.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	// img path
	char *img = argv[1];
	if (engine_start(img) == FAILURE) {
		uerror("%s", "can not load fat12 img");
		return 0;
	}
	engine_shut();
	return 0;
}
