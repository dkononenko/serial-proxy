#include <cstdlib>
#include <sstream>
#include <iomanip>

#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace asio = boost::asio;
namespace pt = boost::property_tree;

class app_t
{
public:
	app_t(asio::io_service& io) : serial_port1(io), serial_port2(io)
	{
		try
		{
			pt::ptree config;
			pt::read_ini("serialproxy.config", config);

			serial_port1.open(config.get<std::string>("COMA.name"));
			serial_port1.set_option(asio::serial_port_base::baud_rate(config.get<int>("COMA.baud_rate")));
			serial_port1.set_option(asio::serial_port_base::character_size(config.get<int>("COMA.character_size")));
			serial_port1.set_option(asio::serial_port_base::stop_bits(stop_bits(config.get<int>("COMA.stop_bits"))));
			serial_port1.set_option(
				asio::serial_port_base::flow_control(flow_control(config.get<std::string>("COMA.flow_control"))));
			serial_port1.set_option(asio::serial_port_base::parity(parity(config.get<std::string>("COMA.parity"))));
			
			serial_port2.open(config.get<std::string>("COMB.name"));
			serial_port2.set_option(asio::serial_port_base::baud_rate(config.get<int>("COMB.baud_rate")));
			serial_port2.set_option(asio::serial_port_base::character_size(config.get<int>("COMB.character_size")));
			serial_port2.set_option(asio::serial_port_base::stop_bits(stop_bits(config.get<int>("COMB.stop_bits"))));
			serial_port2.set_option(
				asio::serial_port_base::flow_control(flow_control(config.get<std::string>("COMB.flow_control"))));
			serial_port2.set_option(asio::serial_port_base::parity(parity(config.get<std::string>("COMB.parity"))));

			serial_port1.async_read_some(asio::buffer(buffer), boost::bind(&app_t::on_read1, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			serial_port2.async_read_some(asio::buffer(buffer), boost::bind(&app_t::on_read2, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
		catch (const std::exception& ex)
		{
			BOOST_LOG_TRIVIAL(error) << ex.what();
		}
	}

private:
	
	void on_read1(const boost::system::error_code &ec, std::size_t length)
	{
		log_buffer(length, " >> ");

		serial_port2.write_some(asio::buffer(buffer, length));

		serial_port1.async_read_some(asio::buffer(buffer), boost::bind(&app_t::on_read1, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	void on_read2(const boost::system::error_code &ec, std::size_t length)
	{
		log_buffer(length, " << ");

		serial_port1.write_some(asio::buffer(buffer, length));

		serial_port2.async_read_some(asio::buffer(buffer), boost::bind(&app_t::on_read2, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	asio::serial_port_base::stop_bits stop_bits(unsigned int value) 
	{
		if (2 == value) {
			return asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::two);
		}
		else {
			return asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one);
		}
	}

	asio::serial_port_base::flow_control flow_control(const std::string &value) 
	{
		if ("software" == value) {
			return asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::software);
		}
		else if ("hardware" == value) {
			return asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::hardware);
		}
		else {
			return asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none);
		}
	}

	asio::serial_port_base::parity parity(const std::string &value) 
	{
		if ("even" == value) {
			return asio::serial_port_base::parity(asio::serial_port_base::parity::even);
		}
		else if ("odd" == value) {
			return asio::serial_port_base::parity(asio::serial_port_base::parity::odd);
		}
		else {
			return asio::serial_port_base::parity(asio::serial_port_base::parity::none);
		}
	}

	void log_buffer(std::size_t length, const std::string& dir)
	{
		std::stringstream ss1, ss2;

		for (std::size_t i = 0; i < length; ++i)
		{
			ss1 << char(buffer[i]);
			ss2 << std::hex << std::setfill('0') << std::setw(2) << (int)buffer[i] << " ";
		}

		BOOST_LOG_TRIVIAL(debug) << dir << ss1.str().c_str();
		BOOST_LOG_TRIVIAL(debug) << dir << ss2.str().c_str() << std::endl;
	}

private:
	asio::serial_port serial_port1, serial_port2;
	uint8_t buffer[4096];
};

int main()
{
	BOOST_LOG_TRIVIAL(info) << "Starting...";

	asio::io_service io;
	app_t app(io);
	
	io.run();

	return EXIT_SUCCESS;
}