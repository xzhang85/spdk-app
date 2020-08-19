
def device_init(client, bdev_name):
    params = {'bdev_name': bdev_name}
    return client.call('device_init', params)


def device_exit(client, bdev_name):
    params = {'bdev_name': bdev_name}
    return client.call('device_exit', params)

def poor_perf1(client, bdev_name, count, _type, size):
    params = {'bdev_name': bdev_name, 'count': count, 'type': _type, 'size': size}
    return client.call('poor_perf1', params)

def poor_perf2(client, bdev_name, count, _type, size):
    params = {'bdev_name': bdev_name, 'count': count, 'type': _type, 'size': size}
    return client.call('poor_perf2', params)
