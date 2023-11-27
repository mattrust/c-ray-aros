//
//  meshloader.c
//  C-ray
//
//  Created by Valtteri Koskivuori on 14.11.2019.
//  Copyright © 2019-2022 Valtteri Koskivuori. All rights reserved.
//

#include <stddef.h>

#include "meshloader.h"
#include "formats/wavefront/wavefront.h"
#include "../fileio.h"
#include "../logging.h"
#include "../../datatypes/mesh.h"

struct mesh_parse_result load_meshes_from_file(const char *file_path, struct file_cache *cache) {
	switch (guess_file_type(file_path)) {
		case obj:
			return parse_wavefront(file_path, cache);
		default:
			logr(warning, "%s: Unknown file type, skipping.\n", file_path);
			return (struct mesh_parse_result){ 0 };
	}
}
