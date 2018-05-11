/*!
 * \file raw_ip_packet_source.h
 *
 * \brief Receives ip frames containing samples in UDP frame encapsulation
 * using a high performance packet capture library (libpcap)
 * \author Javier Arribas jarribas (at) cttc.es
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
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */


#ifndef INCLUDED_RAW_IP_PACKET_SOURCE_H
#define INCLUDED_RAW_IP_PACKET_SOURCE_H

#include <gnuradio/sync_block.h>
#include <boost/thread.hpp>
#include <pcap.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>

class raw_ip_packet_source : virtual public gr::sync_block
{
private:
	boost::mutex d_mutex;

	pcap_t*       descr; //ethernet pcap device descriptor
	int fifo_pipe[2];

	char *fifo_buff;

	int    fifo_read_ptr;
	int    fifo_write_ptr;
	int    fifo_items;
	int d_sock_raw;
	int d_udp_port;
    struct sockaddr_in si_me;
	std::string d_src_device;
	std::string d_origin_address;
	int d_udp_payload_size;
	bool d_fifo_full;

	int d_last_frame_counter;
	int  d_num_rx_errors;


	boost::thread *d_pcap_thread;
	/*!
	 * \brief
	 * Opens the ethernet device using libpcap raw capture mode
	 * If any of these fail, the fuction retuns the error and exits.
	 */
	bool open();

    void my_pcap_loop_thread(pcap_t *pcap_handle);

    void pcap_callback(u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet);

    static void static_pcap_callback(u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet);


public:

    typedef boost::shared_ptr<raw_ip_packet_source> sptr;
    static sptr make(std::string src_device, std::string origin_address, int udp_port, int udp_packet_size);
    raw_ip_packet_source(std::string src_device, std::string origin_address,  int udp_port, int udp_packet_size);
	~raw_ip_packet_source();

	// Where all the action really happens
	int work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);
};

#endif /* INCLUDED_RAW_IP_PACKET_SOURCE_H */

