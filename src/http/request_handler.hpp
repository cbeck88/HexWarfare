//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <string>

namespace http 
{
    namespace server 
    {
        struct reply;
        struct request;

        /// The common handler for all incoming requests.
        class request_handler
        {
        public:
          explicit request_handler();
          void handle_request(const request& req, reply& rep);

        private:
          /// Perform URL-decoding on a string. Returns false if the encoding was
          /// invalid.
          static bool url_decode(const std::string& in, std::string& out);
          
          request_handler(const request_handler&) = delete;
          request_handler& operator=(const request_handler&) = delete;
        };
    }
}
