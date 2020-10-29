#ifndef _IBTRACE_API_H_
#define _IBTRACE_API_H_

/**
 * @enum
 * @brief List of supported module types.
 */
enum {
    IBTRACE_MODULE_IBV = 0,
};

/**
 * ibtrace_timestamp
 * 
 * @brief
 *      This function returns current time in seconds
 * 
 * @retval (value) - time value in seconds
 */
double ibtrace_timestamp(void);

/**
 * ibprof_dump
 * 
 * @brief 
 *      This function dumps all collected data in different formats.
 *      Output can be controlled by environment variables.
 */
void ibtrace_dump(void);



#endif