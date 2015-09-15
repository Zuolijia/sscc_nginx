#ifndef _HEADER_TEST_H_INCLUDED_
#define _HEADER_TEST_H_INCLUDED_

#include <string>
/**
 * @brief HTTP协议头
 *
 * 代表一项HTTP协议头，每个HTTP协议头都包括一个名称和相应的值
 */
struct Header
{
    /**
     * @brief 名称
     */
    std::string name;
    /**
     * @brief 值
     */
    std::string value;
};

#endif