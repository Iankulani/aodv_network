#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_DEVICES 10
#define MAX_NEIGHBORS 10

typedef struct Device {
    char device_id[20];
    float x_pos;
    float y_pos;
    float mobility_range;
    struct Device *neighbors[MAX_NEIGHBORS]; // List of neighbors
    int num_neighbors; // Number of neighbors
    struct {
        char destination_id[20];
        char next_hop[20];
        int hop_count;
    } routing_table[MAX_DEVICES];
    int num_routes; // Number of routes in the routing table
} Device;

// Function to calculate distance between two devices
float distance_to(Device *dev1, Device *dev2) {
    return sqrtf(pow(dev1->x_pos - dev2->x_pos, 2) + pow(dev1->y_pos - dev2->y_pos, 2));
}

// Function to check if two devices can communicate
int can_communicate(Device *dev1, Device *dev2) {
    return distance_to(dev1, dev2) <= dev1->mobility_range;
}

// Function to move the device
void move(Device *device, float dx, float dy) {
    device->x_pos += dx;
    device->y_pos += dy;
    printf("Device %s moved to (%.2f, %.2f)\n", device->device_id, device->x_pos, device->y_pos);
}

// Function to send a Route Request (RREQ)
void send_rreq(Device *source_device, Device *destination_device) {
    printf("Device %s sending RREQ to %s...\n", source_device->device_id, destination_device->device_id);
    for (int i = 0; i < source_device->num_neighbors; i++) {
        Device *neighbor = source_device->neighbors[i];
        if (can_communicate(source_device, neighbor)) {
            printf("Device %s is sending RREQ to neighbor %s\n", source_device->device_id, neighbor->device_id);
            receive_rreq(neighbor, source_device, destination_device);
        }
    }
}

// Function to receive a Route Request (RREQ)
void receive_rreq(Device *device, Device *source_device, Device *destination_device) {
    printf("Device %s received RREQ from %s\n", device->device_id, source_device->device_id);
    if (strcmp(device->device_id, destination_device->device_id) == 0) {
        printf("Device %s found the route to destination %s. Sending RREP...\n", device->device_id, destination_device->device_id);
        send_rrep(device, source_device);
    } else {
        for (int i = 0; i < device->num_neighbors; i++) {
            Device *neighbor = device->neighbors[i];
            if (strcmp(neighbor->device_id, source_device->device_id) != 0 && can_communicate(device, neighbor)) {
                receive_rreq(neighbor, device, destination_device);
            }
        }
    }
}

// Function to send a Route Reply (RREP)
void send_rrep(Device *device, Device *source_device) {
    printf("Device %s sending RREP back to %s\n", device->device_id, source_device->device_id);
    update_routing_table(source_device, device->device_id);
}

// Function to update routing table of the source device
void update_routing_table(Device *device, const char *destination_device_id) {
    strcpy(device->routing_table[device->num_routes].destination_id, destination_device_id);
    strcpy(device->routing_table[device->num_routes].next_hop, device->device_id);
    device->routing_table[device->num_routes].hop_count = 1;
    device->num_routes++;
    printf("Device %s updated routing table: %s -> %s (Hops: %d)\n", device->device_id, device->device_id, destination_device_id, 1);
}

// Function to get the route from the routing table
int get_route(Device *device, Device *destination_device) {
    for (int i = 0; i < device->num_routes; i++) {
        if (strcmp(device->routing_table[i].destination_id, destination_device->device_id) == 0) {
            printf("Route found: %s -> %s (Hops: %d)\n", device->device_id, device->routing_table[i].next_hop, device->routing_table[i].hop_count);
            return 1;
        }
    }
    printf("No route found to device %s\n", destination_device->device_id);
    return 0;
}

// Function to create a new device
Device create_device(const char *device_id, float x_pos, float y_pos, float mobility_range) {
    Device device;
    strcpy(device.device_id, device_id);
    device.x_pos = x_pos;
    device.y_pos = y_pos;
    device.mobility_range = mobility_range;
    device.num_neighbors = 0;
    device.num_routes = 0;
    return device;
}

int main() {
    printf("=========================== AODV-based Ad Hoc Network Routing Simulation ===========================\n");

    // Get user input to create devices
    int num_devices;
    printf("Enter the number of devices:");
    scanf("%d", &num_devices);

    Device devices[MAX_DEVICES];
    for (int i = 0; i < num_devices; i++) {
        char device_id[20];
        float x_pos, y_pos, mobility_range;
        printf("Enter device %d ID:", i + 1);
        scanf("%s", device_id);
        printf("Enter the X coordinate of %s:", device_id);
        scanf("%f", &x_pos);
        printf("Enter the Y coordinate of %s:", device_id);
        scanf("%f", &y_pos);
        printf("Enter the communication range of %s (in meters):", device_id);
        scanf("%f", &mobility_range);
        devices[i] = create_device(device_id, x_pos, y_pos, mobility_range);
    }

    // Setup device neighbors
    for (int i = 0; i < num_devices; i++) {
        for (int j = 0; j < num_devices; j++) {
            if (i != j && can_communicate(&devices[i], &devices[j])) {
                devices[i].neighbors[devices[i].num_neighbors++] = &devices[j];
            }
        }
    }

    // Test communication between two devices
    char source_device_id[20], destination_device_id[20];
    printf("Enter the source device ID to communicate:");
    scanf("%s", source_device_id);
    printf("Enter the destination device ID:");
    scanf("%s", destination_device_id);

    Device *source_device = NULL, *destination_device = NULL;
    for (int i = 0; i < num_devices; i++) {
        if (strcmp(devices[i].device_id, source_device_id) == 0) {
            source_device = &devices[i];
        }
        if (strcmp(devices[i].device_id, destination_device_id) == 0) {
            destination_device = &devices[i];
        }
    }

    if (source_device && destination_device) {
        printf("Source: %s, Destination: %s\n", source_device->device_id, destination_device->device_id);

        // Start route discovery (send RREQ)
        send_rreq(source_device, destination_device);

        // Check if route was discovered
        if (!get_route(source_device, destination_device)) {
            printf("No route found.\n");
        }
    } else {
        printf("Invalid source or destination device IDs.\n");
    }

    return 0;
}
