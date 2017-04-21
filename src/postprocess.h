/**
 * Copyright (c) 2015 harlequin
 * https://github.com/harlequin/viral
 *
 * This file is part of viral.
 *
 * viral is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef POSTPROCESS_H_
#define POSTPROCESS_H_

#define POSTPROCESS_REMOVE_RAR_FILES 0x1
#define POSTPROCESS_REMOVE_NFO_FILES 0x2
#define POSTPROCESS_REMOVE_SFV_FILES 0x4
#define POSTPROCESS_REMOVE_PROOF_FOLDER 0x8
#define POSTPROCESS_REMOVE_SAMPLE_FOLDER 0x16

#define POSTPROCESS_EXTRACT_DIRECTORY "extract"


#include "portable.h"
THREAD_FUNCTION(postprocess);
//void postprocess(const char *name, int post_process_flags);
void list_dir(const char *name, int level);

#endif /* POSTPROCESS_H_ */
