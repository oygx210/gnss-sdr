/*!
 * \file gps_l1_acq_performance_test.cc
 * \brief This class implements an acquisition performance test
 * \author Carles Fernandez-Prades, 2018. cfernandez(at)cttc.cat
 *
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <https://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#include "test_flags.h"
#include "signal_generator_flags.h"
#include <gnuradio/top_block.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

DEFINE_string(config_file_ptest, std::string(""), "File containing the configuration parameters for the position test.");

// ######## GNURADIO BLOCK MESSAGE RECEVER #########
//class AcqPerfTest_msg_rx;
//
//typedef boost::shared_ptr<AcqPerfTest_msg_rx> AcqPerfTest_msg_rx_sptr;
//
//AcqPerfTest_msg_rx_sptr AcqPerfTest_msg_rx_make();
//
//class AcqPerfTest_msg_rx : public gr::block
//{
//private:
//    friend AcqPerfTest_msg_rx_sptr AcqPerfTest_msg_rx_make();
//    void msg_handler_events(pmt::pmt_t msg);
//    AcqPerfTest_msg_rx();
//
//public:
//    int rx_message;
//    ~AcqPerfTest_msg_rx();  //!< Default destructor
//};
//
//
//AcqPerfTest_msg_rx_sptr AcqPerfTest_msg_rx_make()
//{
//    return AcqPerfTest_msg_rx_sptr(new AcqPerfTest_msg_rx());
//}
//
//
//void AcqPerfTest_msg_rx::msg_handler_events(pmt::pmt_t msg)
//{
//    try
//        {
//            long int message = pmt::to_long(msg);
//            rx_message = message;
//        }
//    catch (boost::bad_any_cast& e)
//        {
//            LOG(WARNING) << "msg_handler_telemetry Bad any cast!";
//            rx_message = 0;
//        }
//}
//
//
//AcqPerfTest_msg_rx::AcqPerfTest_msg_rx() : gr::block("AcqPerfTest_msg_rx", gr::io_signature::make(0, 0, 0), gr::io_signature::make(0, 0, 0))
//{
//    this->message_port_register_in(pmt::mp("events"));
//    this->set_msg_handler(pmt::mp("events"), boost::bind(&AcqPerfTest_msg_rx::msg_handler_events, this, _1));
//    rx_message = 0;
//}
//
//
//AcqPerfTest_msg_rx::~AcqPerfTest_msg_rx()
//{
//}
//
//// -----------------------------------------


class AcquisitionPerformanceTest : public ::testing::Test
{
protected:
    AcquisitionPerformanceTest()
    {
        config = std::make_shared<InMemoryConfiguration>();
        item_size = sizeof(gr_complex);
        gnss_synchro = Gnss_Synchro();
        doppler_max = 5000;
        doppler_step = 100;
    }

    ~AcquisitionPerformanceTest()
    {
    }

    std::vector<double> cn0_ = {41.0, 42.0, 43.0};
    int N_iterations = 2;
    void init();
    //void plot_grid();

    int configure_generator(double cn0);
    int generate_signal();
    int configure_receiver(double cn0, unsigned int iter);
    int run_receiver();
    void check_results();
    gr::top_block_sptr top_block;
    Gnss_Synchro gnss_synchro;
    size_t item_size;
    unsigned int doppler_max;
    unsigned int doppler_step;
    std::string implementation = "GPS_L1_CA_PCPS_Acquisition";
    std::shared_ptr<InMemoryConfiguration> config;
    std::shared_ptr<FileConfiguration> config_f;

private:
    std::string generator_binary;
    std::string p1;
    std::string p2;
    std::string p3;
    std::string p4;
    std::string p5;
    std::string p6;

    const double baseband_sampling_freq = static_cast<double>(FLAGS_fs_gen_sps);

    std::string filename_rinex_obs = FLAGS_filename_rinex_obs;
    std::string filename_raw_data = FLAGS_filename_raw_data;

    /*void print_results(const std::vector<double>& east,
        const std::vector<double>& north,
        const std::vector<double>& up);*/

    double compute_stdev_precision(const std::vector<double>& vec);
    double compute_stdev_accuracy(const std::vector<double>& vec, double ref);


    //std::string generated_kml_file;
};


void AcquisitionPerformanceTest::init()
{
    gnss_synchro.Channel_ID = 0;
    gnss_synchro.System = 'G';
    std::string signal = "1C";
    signal.copy(gnss_synchro.Signal, 2, 0);
    gnss_synchro.PRN = 1;
}


int AcquisitionPerformanceTest::configure_generator(double cn0)
{
    // Configure signal generator
    generator_binary = FLAGS_generator_binary;
    int duration_s = 2;

    p1 = std::string("-rinex_nav_file=") + FLAGS_rinex_nav_file;
    if (FLAGS_dynamic_position.empty())
        {
            p2 = std::string("-static_position=") + FLAGS_static_position + std::string(",") + std::to_string(std::min(duration_s * 10, 3000));
        }
    else
        {
            p2 = std::string("-obs_pos_file=") + std::string(FLAGS_dynamic_position);
        }
    p3 = std::string("-rinex_obs_file=") + FLAGS_filename_rinex_obs;               // RINEX 2.10 observation file output
    p4 = std::string("-sig_out_file=") + FLAGS_filename_raw_data;                  // Baseband signal output file. Will be stored in int8_t IQ multiplexed samples
    p5 = std::string("-sampling_freq=") + std::to_string(baseband_sampling_freq);  // Baseband sampling frequency [MSps]
    p6 = std::string("-CN0_dBHz=") + std::to_string(cn0);
    return 0;
}


int AcquisitionPerformanceTest::generate_signal()
{
    pid_t wait_result;
    int child_status;
    std::cout << "Generating signal for " << p6 << "..." << std::endl;
    char* const parmList[] = {&generator_binary[0], &generator_binary[0], &p1[0], &p2[0], &p3[0], &p4[0], &p5[0], &p6[0], NULL};

    int pid;
    if ((pid = fork()) == -1)
        perror("fork error");
    else if (pid == 0)
        {
            execv(&generator_binary[0], parmList);
            std::cout << "Return not expected. Must be an execv error." << std::endl;
            std::terminate();
        }

    wait_result = waitpid(pid, &child_status, 0);
    if (wait_result == -1) perror("waitpid error");
    return 0;
}


int AcquisitionPerformanceTest::configure_receiver(double cn0, unsigned int iter)
{
    if (FLAGS_config_file_ptest.empty())
        {
            config = std::make_shared<InMemoryConfiguration>();
            const int sampling_rate_internal = baseband_sampling_freq;

            const int number_of_taps = 11;
            const int number_of_bands = 2;
            const float band1_begin = 0.0;
            const float band1_end = 0.48;
            const float band2_begin = 0.52;
            const float band2_end = 1.0;
            const float ampl1_begin = 1.0;
            const float ampl1_end = 1.0;
            const float ampl2_begin = 0.0;
            const float ampl2_end = 0.0;
            const float band1_error = 1.0;
            const float band2_error = 1.0;
            const int grid_density = 16;

            const float zero = 0.0;
            const int number_of_channels = 1;
            const int in_acquisition = 1;

            const float threshold = 0.01;
            const float doppler_max = 8000.0;
            const float doppler_step = 500.0;
            const int max_dwells = 1;
            const int tong_init_val = 2;
            const int tong_max_val = 10;
            const int tong_max_dwells = 30;
            const int coherent_integration_time_ms = 1;

            const float pll_bw_hz = 30.0;
            const float dll_bw_hz = 4.0;
            const float early_late_space_chips = 0.5;
            const float pll_bw_narrow_hz = 20.0;
            const float dll_bw_narrow_hz = 2.0;
            const int extend_correlation_ms = 1;

            const int display_rate_ms = 500;
            const int output_rate_ms = 100;

            config->set_property("GNSS-SDR.internal_fs_sps", std::to_string(sampling_rate_internal));

            // Set the assistance system parameters
            config->set_property("GNSS-SDR.SUPL_read_gps_assistance_xml", "false");
            config->set_property("GNSS-SDR.SUPL_gps_enabled", "false");
            config->set_property("GNSS-SDR.SUPL_gps_ephemeris_server", "supl.google.com");
            config->set_property("GNSS-SDR.SUPL_gps_ephemeris_port", std::to_string(7275));
            config->set_property("GNSS-SDR.SUPL_gps_acquisition_server", "supl.google.com");
            config->set_property("GNSS-SDR.SUPL_gps_acquisition_port", std::to_string(7275));
            config->set_property("GNSS-SDR.SUPL_MCC", std::to_string(244));
            config->set_property("GNSS-SDR.SUPL_MNS", std::to_string(5));
            config->set_property("GNSS-SDR.SUPL_LAC", "0x59e2");
            config->set_property("GNSS-SDR.SUPL_CI", "0x31b0");

            // Set the Signal Source
            config->set_property("SignalSource.implementation", "File_Signal_Source");
            config->set_property("SignalSource.filename", "./" + filename_raw_data);
            config->set_property("SignalSource.sampling_frequency", std::to_string(sampling_rate_internal));
            config->set_property("SignalSource.item_type", "ibyte");
            config->set_property("SignalSource.samples", std::to_string(zero));

            // Set the Signal Conditioner
            config->set_property("SignalConditioner.implementation", "Signal_Conditioner");
            config->set_property("DataTypeAdapter.implementation", "Ibyte_To_Complex");
            config->set_property("InputFilter.implementation", "Fir_Filter");
            config->set_property("InputFilter.dump", "false");
            config->set_property("InputFilter.input_item_type", "gr_complex");
            config->set_property("InputFilter.output_item_type", "gr_complex");
            config->set_property("InputFilter.taps_item_type", "float");
            config->set_property("InputFilter.number_of_taps", std::to_string(number_of_taps));
            config->set_property("InputFilter.number_of_bands", std::to_string(number_of_bands));
            config->set_property("InputFilter.band1_begin", std::to_string(band1_begin));
            config->set_property("InputFilter.band1_end", std::to_string(band1_end));
            config->set_property("InputFilter.band2_begin", std::to_string(band2_begin));
            config->set_property("InputFilter.band2_end", std::to_string(band2_end));
            config->set_property("InputFilter.ampl1_begin", std::to_string(ampl1_begin));
            config->set_property("InputFilter.ampl1_end", std::to_string(ampl1_end));
            config->set_property("InputFilter.ampl2_begin", std::to_string(ampl2_begin));
            config->set_property("InputFilter.ampl2_end", std::to_string(ampl2_end));
            config->set_property("InputFilter.band1_error", std::to_string(band1_error));
            config->set_property("InputFilter.band2_error", std::to_string(band2_error));
            config->set_property("InputFilter.filter_type", "bandpass");
            config->set_property("InputFilter.grid_density", std::to_string(grid_density));
            config->set_property("InputFilter.sampling_frequency", std::to_string(sampling_rate_internal));
            config->set_property("InputFilter.IF", std::to_string(zero));
            config->set_property("Resampler.implementation", "Pass_Through");
            config->set_property("Resampler.dump", "false");
            config->set_property("Resampler.item_type", "gr_complex");
            config->set_property("Resampler.sample_freq_in", std::to_string(sampling_rate_internal));
            config->set_property("Resampler.sample_freq_out", std::to_string(sampling_rate_internal));

            // Set the number of Channels
            config->set_property("Channels_1C.count", std::to_string(number_of_channels));
            config->set_property("Channels.in_acquisition", std::to_string(in_acquisition));
            config->set_property("Channel.signal", "1C");

            // Set Acquisition
            config->set_property("Acquisition_1C.implementation", implementation);
            config->set_property("Acquisition_1C.item_type", "gr_complex");
            config->set_property("Acquisition_1C.doppler_max", std::to_string(doppler_max));
            config->set_property("Acquisition_1C.doppler_step", std::to_string(doppler_step));

            config->set_property("Acquisition_1C.threshold", "0.00001");
            //config->set_property("Acquisition_1C.pfa", "0.0");
            config->set_property("Acquisition_1C.use_CFAR_algorithm", "false");

            config->set_property("Acquisition_1C.coherent_integration_time_ms", "1");
            config->set_property("Acquisition_1C.use_bit_transition_flag", "false");

            config->set_property("Acquisition_1C.max_dwells", std::to_string(1));

            config->set_property("Acquisition_1C.repeat_satellite", "false");

            config->set_property("Acquisition_1C.blocking", "true");
            config->set_property("Acquisition_1C.make_two_steps", "false");
            config->set_property("Acquisition_1C.second_nbins", std::to_string(4));
            config->set_property("Acquisition_1C.second_doppler_step", std::to_string(125));
            //if (FLAGS_plot_acq_grid == true)
            //    {
            config->set_property("Acquisition_1C.dump", "true");
            //    }
            //else
            //    {
            //        config->set_property("Acquisition_1C.dump", "false");
            //    }
            std::string dump_file = std::string("./acquisition_") + std::to_string(cn0) + "_" + std::to_string(iter);
            config->set_property("Acquisition_1C.dump_filename", dump_file);

            // Set Tracking
            config->set_property("Tracking_1C.implementation", "GPS_L1_CA_DLL_PLL_Tracking");
            //config->set_property("Tracking_1C.implementation", "GPS_L1_CA_DLL_PLL_C_Aid_Tracking");
            config->set_property("Tracking_1C.item_type", "gr_complex");
            config->set_property("Tracking_1C.dump", "false");
            config->set_property("Tracking_1C.dump_filename", "./tracking_ch_");
            config->set_property("Tracking_1C.pll_bw_hz", std::to_string(pll_bw_hz));
            config->set_property("Tracking_1C.dll_bw_hz", std::to_string(dll_bw_hz));
            config->set_property("Tracking_1C.early_late_space_chips", std::to_string(early_late_space_chips));

            config->set_property("Tracking_1C.pll_bw_narrow_hz", std::to_string(pll_bw_narrow_hz));
            config->set_property("Tracking_1C.dll_bw_narrow_hz", std::to_string(dll_bw_narrow_hz));
            config->set_property("Tracking_1C.extend_correlation_symbols", std::to_string(extend_correlation_ms));

            // Set Telemetry
            config->set_property("TelemetryDecoder_1C.implementation", "GPS_L1_CA_Telemetry_Decoder");
            config->set_property("TelemetryDecoder_1C.dump", "false");

            // Set Observables
            config->set_property("Observables.implementation", "Hybrid_Observables");
            config->set_property("Observables.dump", "false");
            config->set_property("Observables.dump_filename", "./observables.dat");

            // Set PVT
            config->set_property("PVT.implementation", "RTKLIB_PVT");
            config->set_property("PVT.positioning_mode", "PPP_Static");
            config->set_property("PVT.output_rate_ms", std::to_string(output_rate_ms));
            config->set_property("PVT.display_rate_ms", std::to_string(display_rate_ms));
            config->set_property("PVT.dump_filename", "./PVT");
            config->set_property("PVT.nmea_dump_filename", "./gnss_sdr_pvt.nmea");
            config->set_property("PVT.flag_nmea_tty_port", "false");
            config->set_property("PVT.nmea_dump_devname", "/dev/pts/4");
            config->set_property("PVT.flag_rtcm_server", "false");
            config->set_property("PVT.flag_rtcm_tty_port", "false");
            config->set_property("PVT.rtcm_dump_devname", "/dev/pts/1");
            config->set_property("PVT.dump", "false");
            config->set_property("PVT.rinex_version", std::to_string(2));
            config->set_property("PVT.iono_model", "OFF");
            config->set_property("PVT.trop_model", "OFF");
            config->set_property("PVT.AR_GPS", "PPP-AR");

            config_f = 0;
        }
    else
        {
            config_f = std::make_shared<FileConfiguration>(FLAGS_config_file_ptest);
            config = 0;
        }
    return 0;
}


int AcquisitionPerformanceTest::run_receiver()
{
    std::shared_ptr<ControlThread> control_thread;
    if (FLAGS_config_file_ptest.empty())
        {
            control_thread = std::make_shared<ControlThread>(config);
        }
    else
        {
            control_thread = std::make_shared<ControlThread>(config_f);
        }

    // start receiver
    try
        {
            control_thread->run();
        }
    catch (const boost::exception& e)
        {
            std::cout << "Boost exception: " << boost::diagnostic_information(e);
        }
    catch (const std::exception& ex)
        {
            std::cout << "STD exception: " << ex.what();
        }
    return 0;
}


TEST_F(AcquisitionPerformanceTest, PdvsCn0)
{
    for (std::vector<double>::const_iterator it = cn0_.cbegin(); it != cn0_.cend(); ++it)
        {
            // Set parameter to sweep

            // Do N_iterations of the experiment
            for (unsigned iter = 0; iter < N_iterations; iter++)
                {
                    // Configure the signal generator
                    configure_generator(*it);

                    // Generate signal raw signal samples and observations RINEX file
                    generate_signal();

                    // Configure the receiver
                    configure_receiver(*it, iter);

                    // Run it
                    run_receiver();

                    // Read and store reference data and results
                }
        }

    // Compute results
}

//TEST_F(AcquisitionPerformanceTest, ValidationOfResults)
//{
//    std::chrono::time_point<std::chrono::system_clock> start, end;
//    std::chrono::duration<double> elapsed_seconds(0.0);
//    top_block = gr::make_top_block("Acquisition test");
//
//    double expected_delay_samples = 524;
//    double expected_doppler_hz = 1680;
//
//    init();
//
//    if (FLAGS_plot_acq_grid == true)
//        {
//            std::string data_str = "./tmp-acq-gps1";
//            if (boost::filesystem::exists(data_str))
//                {
//                    boost::filesystem::remove_all(data_str);
//                }
//            boost::filesystem::create_directory(data_str);
//        }
//
//    std::shared_ptr<GpsL1CaPcpsAcquisition> acquisition = std::make_shared<GpsL1CaPcpsAcquisition>(config.get(), "Acquisition_1C", 1, 0);
//    boost::shared_ptr<AcqPerfTest_msg_rx> msg_rx = AcqPerfTest_msg_rx_make();
//
//    ASSERT_NO_THROW({
//        acquisition->set_channel(1);
//    }) << "Failure setting channel.";
//
//    ASSERT_NO_THROW({
//        acquisition->set_gnss_synchro(&gnss_synchro);
//    }) << "Failure setting gnss_synchro.";
//
//    ASSERT_NO_THROW({
//        acquisition->set_threshold(0.001);
//    }) << "Failure setting threshold.";
//
//    ASSERT_NO_THROW({
//        acquisition->set_doppler_max(doppler_max);
//    }) << "Failure setting doppler_max.";
//
//    ASSERT_NO_THROW({
//        acquisition->set_doppler_step(doppler_step);
//    }) << "Failure setting doppler_step.";
//
//    ASSERT_NO_THROW({
//        acquisition->connect(top_block);
//    }) << "Failure connecting acquisition to the top_block.";
//
//    ASSERT_NO_THROW({
//        std::string path = std::string(TEST_PATH);
//        std::string file = path + "signal_samples/GPS_L1_CA_ID_1_Fs_4Msps_2ms.dat";
//        const char* file_name = file.c_str();
//        gr::blocks::file_source::sptr file_source = gr::blocks::file_source::make(sizeof(gr_complex), file_name, false);
//        top_block->connect(file_source, 0, acquisition->get_left_block(), 0);
//        top_block->msg_connect(acquisition->get_right_block(), pmt::mp("events"), msg_rx, pmt::mp("events"));
//    }) << "Failure connecting the blocks of acquisition test.";
//
//    acquisition->set_local_code();
//    acquisition->set_state(1);  // Ensure that acquisition starts at the first sample
//    acquisition->init();
//
//    EXPECT_NO_THROW({
//        start = std::chrono::system_clock::now();
//        top_block->run();  // Start threads and wait
//        end = std::chrono::system_clock::now();
//        elapsed_seconds = end - start;
//    }) << "Failure running the top_block.";
//
//    unsigned long int nsamples = gnss_synchro.Acq_samplestamp_samples;
//    std::cout << "Acquired " << nsamples << " samples in " << elapsed_seconds.count() * 1e6 << " microseconds" << std::endl;
//    ASSERT_EQ(1, msg_rx->rx_message) << "Acquisition failure. Expected message: 1=ACQ SUCCESS.";
//
//    double delay_error_samples = std::abs(expected_delay_samples - gnss_synchro.Acq_delay_samples);
//    float delay_error_chips = static_cast<float>(delay_error_samples * 1023 / 4000);
//    double doppler_error_hz = std::abs(expected_doppler_hz - gnss_synchro.Acq_doppler_hz);
//
//    EXPECT_LE(doppler_error_hz, 666) << "Doppler error exceeds the expected value: 666 Hz = 2/(3*integration period)";
//    EXPECT_LT(delay_error_chips, 0.5) << "Delay error exceeds the expected value: 0.5 chips";
//
//    /*if (FLAGS_plot_acq_grid == true)
//        {
//            plot_grid();
//        }*/
//}
