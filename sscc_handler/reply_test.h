#include <string>
#include <vector>
#include "header_test.h"

struct Reply
{
    /**
     * @brief 回应的状态号
     *
     * 正常是ok（200）
     */
    enum StatusType
    {
        ok = 200,
        created = 201,
        accepted = 202,
        no_content = 204,
        multiple_choices = 300,
        moved_permanently = 301,
        moved_temporarily = 302,
        not_modified = 304,
        bad_request = 400,
        unauthorized = 401,
        forbidden = 403,
        not_found = 404,
        internal_server_error = 500,
        not_implemented = 501,
        bad_gateway = 502,
        service_unavailable = 503
    } status;

    /**
     * @brief HTTP头
     */
    std::vector<Header> headers;

    /**
     * @brief 内容
     */
    std::string content;

    Reply(){
        status = created;
        content = "";
    }
};