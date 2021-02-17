#include "module_proba_chris.h"
#include <fstream>
#include "chris_reader.h"
#include "modules/proba/ccsds/demuxer.h"
#include "modules/proba/ccsds/vcdu.h"
#include "logger.h"
#include <filesystem>

// Return filesize
size_t getFilesize(std::string filepath);

namespace proba
{
    namespace chris
    {
        ProbaCHRISDecoderModule::ProbaCHRISDecoderModule(std::string input_file, std::string output_file_hint, std::map<std::string, std::string> parameters) : ProcessingModule(input_file, output_file_hint, parameters)
        {
        }

        void ProbaCHRISDecoderModule::process()
        {
            size_t filesize = getFilesize(d_input_file);
            std::ifstream data_in(d_input_file, std::ios::binary);

            std::string directory = d_output_file_hint.substr(0, d_output_file_hint.rfind('/')) + "/CHRIS";

            if (!std::filesystem::exists(directory))
                std::filesystem::create_directory(directory);

            logger->info("Using input frames " + d_input_file);
            logger->info("Decoding to " + directory);

            time_t lastTime = 0;

            uint8_t buffer[1279];

            // CCSDS Demuxer
            libccsds::Demuxer ccsdsDemuxer(1103, false);

            CHRISReader chris_reader(directory);

            logger->info("Demultiplexing and deframing...");

            while (!data_in.eof())
            {
                // Read buffer
                data_in.read((char *)buffer, 1279);

                int vcid = libccsds::parseVCDU(buffer).vcid;

                if (vcid == 1)
                {
                    std::vector<libccsds::CCSDSPacket> pkts = ccsdsDemuxer.work(buffer);

                    if (pkts.size() > 0)
                    {
                        for (libccsds::CCSDSPacket pkt : pkts)
                        {
                            if (pkt.header.apid == 2047)
                                continue;

                            if (pkt.header.apid == 0)
                            {
                                int mode_marker = pkt.payload[9] & 0x03;
                                if (mode_marker != 1)
                                    chris_reader.work(pkt);

                                //if (pkt.payload.size() >= 11538)
                                //    data_out.write((char *)pkt.payload.data(), 11538);
                            }
                        }
                    }
                }

                if (time(NULL) % 10 == 0 && lastTime != time(NULL))
                {
                    lastTime = time(NULL);
                    logger->info("Progress " + std::to_string(round(((float)data_in.tellg() / (float)filesize) * 1000.0f) / 10.0f) + "%");
                }
            }

            data_in.close();

            chris_reader.save();

            d_output_files = chris_reader.all_images;
        }

        std::string ProbaCHRISDecoderModule::getID()
        {
            return "proba_chris";
        }

        std::vector<std::string> ProbaCHRISDecoderModule::getParameters()
        {
            return {};
        }

        std::shared_ptr<ProcessingModule> ProbaCHRISDecoderModule::getInstance(std::string input_file, std::string output_file_hint, std::map<std::string, std::string> parameters)
        {
            return std::make_shared<ProbaCHRISDecoderModule>(input_file, output_file_hint, parameters);
        }
    } // namespace chris
} // namespace proba