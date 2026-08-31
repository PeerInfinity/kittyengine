//
// This is a tool file that gets accessed within the Rapt framework.
// You should never include this file manually, include "rapt.h" instead.
//

#include "os_headers.h"
//#include "rapt_defines.h"

#ifdef LEAKTESTING
#include "leaktester.h"
#endif

#include "util_core.h"
#include "rapt_smartpointer.h"
#include "rapt_point.h"
#include "rapt_random.h"
#include "rapt_object.h"
#include "rapt_list.h"
#include "rapt_string.h"
#include "rapt_rect.h"
#include "rapt_math.h"
#include "rapt_console.h"
#include "rapt_color.h"
#include "rapt_iobuffer.h"
#include "rapt_sprite.h"
#include "rapt_image.h"
#include "rapt_utils.h"
#include "rapt_font.h"
#include "rapt_CPU.h"
#include "rapt_settings.h"
//#include "rapt_shaders.h"
//#include "rapt_model.h"

#ifdef _WIN32
#define aligned_alloc(align,size) _aligned_malloc(size,align)
#define aligned_free _aligned_free
#else
#define aligned_free free
#endif

