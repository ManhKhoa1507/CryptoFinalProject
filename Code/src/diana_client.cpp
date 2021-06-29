//
//  diana_client.cpp
//  diana
//
//  Created by Raphael Bost on 20/07/2016.
//  Copyright © 2016 Raphael Bost. All rights reserved.
//

#include "sse/runners/diana/client_runner.hpp"

#include <sse/schemes/utils/db_generator.hpp>
#include <sse/schemes/utils/logger.hpp>

#include <sse/crypto/utils.hpp>

#include <cstdio>
#include <unistd.h>

#include <iostream>
using std::string;
using std::cout;
using std::endl; 

#include <list>
#include <mutex>

__thread std::list<std::pair<std::string, uint64_t>>* g_diana_buffer_list_
    = nullptr;

void RunDatabase(std::string client_db)
{
    // Run the given database
    // If empty run the default database : test.dcdb
    if (client_db.empty()) {
        sse::logger::logger()->warn(
            "Client database not specified. Using \'test.dcdb\' by default");
        client_db = "test.dcdb";
    } 

    // Run the given database     
    else {
        sse::logger::logger()->info("Running client with database "
                                    + client_db);
    }
}

int main(int argc, char** argv)
{
    sse::logger::set_logging_level(spdlog::level::info);
    sse::Benchmark::set_benchmark_file("benchmark_diana_client.out");

    sse::crypto::init_crypto_lib();

    opterr = 0;
    int c;

    std::list<std::string> input_files;
    std::list<std::string> keywords;
    std::string            client_db;
    uint32_t               rnd_entries_count = 0;

    bool print_results = true;

    while ((c = getopt(argc, argv, "l:b:dr:q")) != -1) {
        switch (c) {
            
        // Option -l to load the file.json : load the reversed index file.json
        // and add it to the database
        case 'l':
            input_files.emplace_back(optarg);
            break;

        // Option -b to use the file as the client database
        case 'b':
            client_db = std::string(optarg);
            break;

        // Option -d to load the default file for debugging
        // Input_files.push_back("/Volumes/Storage/WP_Inverted/inverted_index_all_sizes/inverted_index_10000.json");
        case 'd':
            input_files.emplace_back(
                "~/Desktop/Code/DoAnCrypto/inverted_index.json");
            break;

        // Option -q to check if print the result or not
        case 'q':
            print_results = false;
            break;

        // Option -r (count) to generate the database with count entries
        case 'r':
            rnd_entries_count = static_cast<uint32_t>(
                std::stod(std::string(optarg), nullptr));
            // atol(optarg);
            break;

        // Handle with invalid options
        case '?':

            // Without argument
            if (optopt == 'l' || optopt == 'b' || optopt == 'o' || optopt == 'i'
                || optopt == 't' || optopt == 'r') {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            }

            // Unknown options
            else if (isprint(optopt) != 0) {
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            }

            // Unknown the option character
            else {
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
            }

            return 1;

        default:
            exit(-1);
        }
    }


    for (int index = optind; index < argc; index++) {
        keywords.emplace_back(argv[index]);
    }

    // Run the database
    RunDatabase(client_db);

    // Run the client in localhost:4240
    std::unique_ptr<sse::diana::DianaClientRunner> client_runner;

    std::shared_ptr<grpc::Channel> channel(grpc::CreateChannel(
        "0.0.0.0:4241", grpc::InsecureChannelCredentials()));
    client_runner.reset(new sse::diana::DianaClientRunner(channel, client_db));

    // Load the default file for debugging
    for (std::string& path : input_files) {
        sse::logger::logger()->info("Load file " + path);
        client_runner->load_inverted_index(path);
        sse::logger::logger()->info("Done loading file " + path);
    }

    if (rnd_entries_count > 0) {
        sse::logger::logger()->info("Randomly generating database with {} docs",
                                    rnd_entries_count);

        auto gen_callback = [&client_runner](const std::string& s, size_t i) {
            if (g_diana_buffer_list_ == nullptr) {
                g_diana_buffer_list_
                    = new std::list<std::pair<std::string, uint64_t>>();
            }
            g_diana_buffer_list_->push_back(std::make_pair(s, i));

            if (g_diana_buffer_list_->size() >= 50) {
                client_runner->insert_in_session(*g_diana_buffer_list_);

                g_diana_buffer_list_->clear();
            }
        };

        // Update the session 
        client_runner->start_update_session();
        sse::sophos::gen_db(rnd_entries_count, gen_callback);
        client_runner->end_update_session();
    }

    // Searching the keyword 
    for (std::string& kw : keywords) {
        std::cout << "-------------- Search --------------" << std::endl;

        std::mutex out_mtx;
        bool       first = true;

        auto print_callback = [&out_mtx, &first, print_results](uint64_t res) {
            if (print_results)
             {
                out_mtx.lock();

                if (!first) 
                {
                    std::cout << ", ";
                }
                first = false;
                std::cout << res;

                out_mtx.unlock();
            }
        };

        std::cout << "Search results: \n{";

        auto res = client_runner->search(kw, print_callback);

        std::cout << "}" << std::endl;
    }

    client_runner.reset();
    sse::crypto::cleanup_crypto_lib();

    return 0;
}
