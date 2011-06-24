#include <types.h>

/* update train's location at current time */
int update_train_location( Train_data *train );
/* update train's speed (also location) when hit new sensor */
int update_train_speed( Train_data* train, track_node* new_sensor, track_node* track );

