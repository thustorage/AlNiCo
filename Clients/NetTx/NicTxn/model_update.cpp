
#include "model_update.h"

feature_partitiont_t feature_partition;

void init_feature_partition()
{
    feature_partition.offset[0] = 0;
    feature_partition.size[0] = feature_size_512base[0] ;
    for (int i = 1; i < 12; i++) {
        feature_partition.offset[i] = feature_partition.offset[i - 1] + feature_size_512base[i-1] ;
        feature_partition.size[i] = feature_size_512base[i];
    }
}