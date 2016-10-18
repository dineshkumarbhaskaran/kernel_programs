#ifndef __SRD_COMMON_H__
#define __SRD_COMMON_H__

#define SRD_VERSION "0.1"

#ifdef DEBUG 
#define trace() pr_info ("%s@%s:%d\n", __FUNCTION__, __FILE__, __LINE__);

#else
#define trace()
#endif 

#endif //__SRD_COMMON_H__
