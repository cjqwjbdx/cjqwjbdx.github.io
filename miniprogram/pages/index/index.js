// pages/index/index.js
import { bleManager } from '../../utils/ble.js'

const app = getApp()

Page({
  data: {
    devices: [],
    connected: false,
    connectedDeviceName: '',
    deviceId: '',
    searching: false,
    statusText: '未连接'
  },

  onLoad() {
    this.initBLE()
  },

  onUnload() {
    bleManager.disconnect()
  },

  async initBLE() {
    const success = await bleManager.initBluetooth()
    if (success) {
      this.setData({
        statusText: '准备就绪'
      })
    }
  },

  // 开始搜索
  async startSearch() {
    if (this.data.searching) return

    this.setData({
      devices: [],
      searching: true,
      statusText: '搜索中...'
    })

    await bleManager.startSearch()

    // 监听设备发现
    bleManager.onConnectionStateChange((status, success, data) => {
      if (status === 'deviceFound') {
        this.setData({ devices: data })
      } else if (status === 'searching' && !success) {
        this.setData({
          searching: false,
          statusText: '搜索结束'
        })
      }
    })
  },

  // 连接设备
  async connectDevice(e) {
    const device = e.currentTarget.dataset.device

    await bleManager.stopSearch()

    this.setData({
      statusText: '连接中...'
    })

    // 监听连接状态
    bleManager.onConnectionStateChange((status, success, data) => {
      if (status === 'connected' && success) {
        this.setData({
          connected: true,
          connectedDeviceName: device.name,
          deviceId: data,
          statusText: '已连接'
        })
        app.globalData.deviceId = data
        app.globalData.serviceId = bleManager.serviceId
        app.globalData.characteristicId = bleManager.characteristicId
        app.globalData.connected = true
      }
    })

    await bleManager.connect(device.deviceId)
  },

  // 断开连接
  async disconnect() {
    await bleManager.disconnect()
    this.setData({
      connected: false,
      connectedDeviceName: '',
      deviceId: '',
      statusText: '未连接'
    })
    app.globalData.connected = false
  },

  get statusClass() {
    if (this.data.connected) return 'connected'
    if (this.data.searching) return 'searching'
    return 'disconnected'
  }
})
