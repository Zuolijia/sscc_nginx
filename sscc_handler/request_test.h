#include <map>
#include <string>
#include <vector>
#include "header_test.h"

struct Request
{
    /** @brief 存放Cookie的类型 */
    typedef std::multimap<std::string, std::string> CookieMap;
    /** @brief 存放HTTP查询参数的类型 */
    typedef std::multimap<std::string, std::string> QueryMap;

    /** @brief 浏览器地址 */
    std::string           remoteAddress;
    /** @brief 浏览器端口 */
    unsigned short        remotePort;
    /** @brief 模式（GET/POST等） */
    std::string           method;
    /** @brief 用户请求的完整URI，含查询串 */
    std::string           uri;
    /** @brief 请求的数据 */
    std::vector<char>     body;
    /** @brief HTTP主版本号 */
    int                   httpVersionMajor;
    /** @brief HTTP副版本号 */
    int                   httpVersionMinor;
    /** @brief HTTP头 */
    std::vector<Header>   headers;

    /** @brief Cookie */
    CookieMap             cookies;
    /** @brief 查询参数 */
    QueryMap              querys;
    /** @brief 用户请求的路径，不含查询串 */
    std::string           requestPath;
    std::string           contentType;

    //ContinuationWeakPtr   continuation;
    Request(){
        remoteAddress = "Default";
        remotePort = 80;
        method = "Default";
        uri = "Default";
        httpVersionMinor = 0;
        httpVersionMajor = 0;
        requestPath = "Default";
        contentType = "Default";
    }
};