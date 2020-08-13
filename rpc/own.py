
def device_init(client, bdev_name):
    params = {'bdev_name': bdev_name}
    return client.call('device_init', params)


def device_exit(client, bdev_name):
    params = {'bdev_name': bdev_name}
    return client.call('device_exit', params)

def poor_perf(client, bdev_name, count):
    params = {'bdev_name': bdev_name, 'count': count}
    return client.call('poor_perf', params)
