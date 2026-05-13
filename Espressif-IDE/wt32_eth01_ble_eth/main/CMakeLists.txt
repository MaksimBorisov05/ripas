# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS main.c app.c eth.c ble.c tcp_client.c transport_tcp.c monotonic.c time_sync.c ble_buffer.c packet_builder.c scan.pb.c protobuf_builder.c batch_builder.c# list the source files of this component
    INCLUDE_DIRS "."    # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES         # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
#bt esp_eth esp_netif lwip nvs_flash