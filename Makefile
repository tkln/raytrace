CFLAGS=-O2 -g -lm
raytrace: raytrace.c fb.h ray.h vecmat/vec3f.h
	${CC} ${CFLAGS} $< -o $@
