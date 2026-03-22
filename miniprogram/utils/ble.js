// 蓝牙通信工具类
const SERVICE_UUID = '0000FFE0-0000-1000-8000-00805F9B34FB' // ESP32 BLE服务UUID
const CHARACTERISTIC_UUID = '0000FFE1-0000-1000-8000-00805F9B34FB' // 写特征UUID

class BLEManager {
  constructor() {
    this.adapterAvailable = false
    this.searching = false
    this.devices = []
    this.connected = false
    this.deviceId = null
    this.serviceId = null
    this.characteristicId = null
    this.onDataCallback = null
    this.onStatusCallback = null
  }

  // 初始化蓝牙适配器
  async initBluetooth() {
    try {
      await wx.openBluetoothAdapter()
      this.adapterAvailable = true
      console.log('蓝牙适配器已初始化')
      this._notifyStatus('init', true)
      return true
    } catch (error) {
      console.error('蓝牙初始化失败:', error)
      this._notifyStatus('init', false, error.errMsg)
      wx.showToast({
        title: '请开启蓝牙',
        icon: 'none'
      })
      return false
    }
  }

  // 开始搜索设备
  async startSearch() {
    if (!this.adapterAvailable) {
      const success = await this.initBluetooth()
      if (!success) return
    }

    try {
      this.searching = true
      this.devices = []
      this._notifyStatus('searching', true)

      await wx.startBluetoothDevicesDiscovery({
        allowDuplicatesKey: false
      })

      wx.onBluetoothDeviceFound((res) => {
        const device = res.devices[0]
        if (device.name || device.localName) {
          const exists = this.devices.find(d => d.deviceId === device.deviceId)
          if (!exists) {
            this.devices.push({
              deviceId: device.deviceId,
              name: device.name || device.localName || '未知设备',
              rssi: device.RSSI
            })
            this._notifyStatus('deviceFound', this.devices)
          }
        }
      })

      // 10秒后停止搜索
      setTimeout(() => {
        this.stopSearch()
      }, 10000)

    } catch (error) {
      console.error('搜索失败:', error)
      this.searching = false
      this._notifyStatus('searching', false, error.errMsg)
    }
  }

  // 停止搜索
  async stopSearch() {
    try {
      await wx.stopBluetoothDevicesDiscovery()
      this.searching = false
      this._notifyStatus('searching', false)
    } catch (error) {
      console.error('停止搜索失败:', error)
    }
  }

  // 连接设备
  async connect(deviceId) {
    try {
      wx.showLoading({ title: '连接中...' })

      await wx.createBLEConnection({
        deviceId: deviceId
      })

      this.deviceId = deviceId
      this.connected = true
      this._notifyStatus('connected', true, deviceId)

      // 设置MTU
      try {
        await wx.setBLEMTU({
          deviceId: deviceId,
          mtu: 512
        })
      } catch (e) {
        console.log('MTU设置失败，使用默认值')
      }

      // 获取服务列表
      const services = await wx.getBLEDeviceServices({
        deviceId: deviceId
      })

      // 查找目标服务
      const service = services.services.find(s => 
        s.uuid.toLowerCase() === SERVICE_UUID.toLowerCase()
      )

      if (!service) {
        throw new Error('未找到蓝牙服务')
      }

      this.serviceId = service.uuid

      // 获取特征值
      const characteristics = await wx.getBLEDeviceCharacteristics({
        deviceId: deviceId,
        serviceId: service.uuid
      })

      const characteristic = characteristics.characteristics.find(c =>
        c.uuid.toLowerCase() === CHARACTERISTIC_UUID.toLowerCase()
      )

      if (!characteristic) {
        throw new Error('未找到特征值')
      }

      this.characteristicId = characteristic.uuid

      // 监听数据接收
      wx.onBLECharacteristicValueChange((res) => {
        if (this.onDataCallback) {
          this.onDataCallback(this._arrayBufferToString(res.value))
        }
      })

      // 启用通知
      await wx.notifyBLECharacteristicValueChange({
        deviceId: deviceId,
        serviceId: service.uuid,
        characteristicId: characteristic.uuid,
        state: true
      })

      wx.hideLoading()
      wx.showToast({
        title: '连接成功',
        icon: 'success'
      })

    } catch (error) {
      wx.hideLoading()
      console.error('连接失败:', error)
      this.connected = false
      this._notifyStatus('connected', false, error.errMsg)
      wx.showToast({
        title: '连接失败',
        icon: 'none'
      })
    }
  }

  // 断开连接
  async disconnect() {
    try {
      if (this.deviceId) {
        await wx.closeBLEConnection({
          deviceId: this.deviceId
        })
      }
      this.deviceId = null
      this.serviceId = null
      this.characteristicId = null
      this.connected = false
      this._notifyStatus('disconnected')
    } catch (error) {
      console.error('断开连接失败:', error)
    }
  }

  // 发送数据
  async sendData(data) {
    if (!this.connected || !this.deviceId || !this.serviceId || !this.characteristicId) {
      throw new Error('设备未连接')
    }

    try {
      const buffer = this._stringToArrayBuffer(data)
      await wx.writeBLECharacteristicValue({
        deviceId: this.deviceId,
        serviceId: this.serviceId,
        characteristicId: this.characteristicId,
        value: buffer
      })
      console.log('发送数据:', data)
    } catch (error) {
      console.error('发送失败:', error)
      throw error
    }
  }

  // 监听连接状态变化
  onConnectionStateChange(callback) {
    this.onStatusCallback = callback
  }

  // 监听数据接收
  onDataReceived(callback) {
    this.onDataCallback = callback
  }

  // 关闭蓝牙适配器
  async close() {
    await this.stopSearch()
    await this.disconnect()
    await wx.closeBluetoothAdapter()
    this.adapterAvailable = false
  }

  // 工具方法：字符串转ArrayBuffer
  _stringToArrayBuffer(str) {
    const buffer = new ArrayBuffer(str.length)
    const view = new Uint8Array(buffer)
    for (let i = 0; i < str.length; i++) {
      view[i] = str.charCodeAt(i)
    }
    return buffer
  }

  // 工具方法：ArrayBuffer转字符串
  _arrayBufferToString(buffer) {
    const uint8Array = new Uint8Array(buffer)
    let str = ''
    for (let i = 0; i < uint8Array.length; i++) {
      str += String.fromCharCode(uint8Array[i])
    }
    return str
  }

  // 通知状态变化
  _notifyStatus(status, success, data) {
    if (this.onStatusCallback) {
      this.onStatusCallback(status, success, data)
    }
  }
}

// 导出单例
export const bleManager = new BLEManager()
