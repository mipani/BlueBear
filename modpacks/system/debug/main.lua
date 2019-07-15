local path = ...
local fps = bluebear.util.get_fps()

dofile( path..'/profiler.lua' )

local Panel = {
  animation_in_progress = false,
  ANIMATIONS = {
    OPEN = {
      fps = fps,
      duration = fps / 4,
      keyframes = {
        [ 0.0 ] = {
          frames = {
            top = -450
          }
        },
        [ fps / 4 ] = {
          interpolate = true,
          frames = {
            top = 0
          }
        }
      }
    },
    CLOSE = {
      fps = fps,
      duration = fps / 4,
      keyframes = {
        [ 0.0 ] = {
          frames = {
            top = 0
          }
        },
        [ fps / 4 ] = {
          interpolate = true,
          frames = {
            top = -450
          }
        }
      }
    }
  },
  LOG_MESSAGE_TEMPLATE = [[
  <Layout class="-bb-log-message">
    <Text class="-bb-date %s">%s</Text>
    <Spacer class="buffer-log"></Spacer>
    <Text class="-bb-message">%s</Text>
  </Layout>
  ]],
  LEVEL_CLASSES = {
    [ 'd' ] = 'debug',
    [ 'i' ] = 'info',
    [ 'w' ] = 'warning',
    [ 'e' ] = 'error'
  },
  pane = nil,
  scrollback_bin = nil,
  scroll = nil,
  input_field = nil,
  system_event = nil,
  message_queue = nil,
  queue_waiting = false
}

function Panel:init()
  bluebear.gui.load_stylesheet( { path..'/panel.style', path..'/window.style', path..'/window_test.style' } )
  self.pane = bluebear.gui.load_xml( path..'/panel.xml' )[ 1 ]
  bluebear.gui.root_element:add_child( self.pane )

  bluebear.event.register_key( '`', bluebear.util.bind( self.toggle, self ) )
  bluebear.event.register_key( 'w', function()
    self.window = bluebear.gui.load_xml( path..'/window.xml' )[ 1 ]
    bluebear.gui.root_element:add_child( self.window )
  end )
  bluebear.event.register_key( 'a', function()
    bluebear.gui.root_element:add_child(
      bluebear.gui.load_xml( path..'/window_test.xml' )[ 1 ]
    )
  end )
  self.pane:set_style_property( 'top', -450 )

  self.scrollback_bin = self.pane:get_elements_by_class( { '-bb-scrollback-bin' } )[ 1 ]
  self.input_field = self.pane:get_elements_by_class( { '-bb-terminal-text-input' } )[ 1 ]
  self.message_queue = {}
  self.queue_waiting = false
  self.system_event = bluebear.event.register_system_event( 'message-logged', bluebear.util.bind( self.receive_message, self ) )
  self.pane
    :get_elements_by_class( { '-bb-terminal-clear-button' } )[ 1 ]
    :register_input_event(
      'mouse-up',
      bluebear.util.bind( self.clear, self )
    )

  self.pane
    :get_elements_by_class( { '-bb-terminal-execute-button' } )[ 1 ]
    :register_input_event(
      'mouse-up',
      bluebear.util.bind( self.execute, self )
    )
end

function Panel:close()
end

function Panel:load( jsonString )
  local x = bluebear.util.json.decode( jsonString )
end

function Panel:save()
end

function Panel:execute( event )
  local contents = self.input_field:get_contents()
  self.input_field:set_contents( "" )

  assert( load( contents ) )()
end

function Panel:clear( event )
  local children = self.scrollback_bin:get_children()
  local tab = {}

  for i, v in ipairs( children ) do
    table.insert( tab, v )
  end

  self.scrollback_bin:remove( tab )

  self.pane:get_elements_by_class( { '-bb-scrollback' } )[ 1 ]:set_x( 0 )
  self.pane:get_elements_by_class( { '-bb-scrollback' } )[ 1 ]:set_y( 0 )
end

function Panel:receive_message( message )
  bluebear.event.unregister_system_event( 'message-logged', self.system_event )

  local segment = bluebear.util.split( message, '[' )
  table.insert( self.message_queue,
    bluebear.gui.load_xml(
      string.format(
        self.LOG_MESSAGE_TEMPLATE,
        self.LEVEL_CLASSES[ string.sub( segment[ 1 ], 2, 2 ) ],
        bluebear.util.sanitize_xml( segment[ 1 ] ),
        bluebear.util.sanitize_xml( '['..segment[ 2 ] )
      )
    , false )[ 1 ]
  )

  -- Insert at the next frame
  if self.queue_waiting == false then
    self.queue_waiting = true
    bluebear.engine.queue_callback( 1, bluebear.util.bind( self.insert_queue, self ) )
  end

  self.system_event = bluebear.event.register_system_event(
    'message-logged',
    bluebear.util.bind( self.receive_message, self )
  )
end

function Panel:insert_queue()
  -- Prevent loopback
  bluebear.event.unregister_system_event( 'message-logged', self.system_event )

  self.queue_waiting = false
  self.scrollback_bin:add_child( self.message_queue )
  self.message_queue = {}

  self:trim_elements()

  -- Restore
  self.system_event = bluebear.event.register_system_event(
    'message-logged',
    bluebear.util.bind( self.receive_message, self )
  )
end

function Panel:trim_elements()
  local cap = bluebear.config.get_int_value( "debug_console_trim" )
  local children = self.scrollback_bin:get_children()
  local difference = #children - cap

  if difference > 0 then
    -- remove items at the beginning of the array
    local to_remove = {}
    for i = 1, difference do
      table.insert( to_remove, children[ i ] )
    end

    self.scrollback_bin:remove( to_remove )
  end
end

function Panel:toggle()
  if self.animation_in_progress == false then
    self.animation_in_progress = true
    local closed = self.pane:get_style_property( 'top' ) == -450

    if closed == true then
      -- Closed to Open
      self.pane:attach_animation( self.ANIMATIONS.OPEN, function()
        self.animation_in_progress = false
        self.pane:set_style_property( 'top', 0 )
      end )
    else
      -- Open to Closed
      self.pane:attach_animation( self.ANIMATIONS.CLOSE, function()
        self.animation_in_progress = false
        self.pane:set_style_property( 'top', -450 )
      end )
    end
  end
end

bluebear.entity.register_component( 'system.debug.terminal', Panel )
bluebear.entity.register_entity( 'system.debug.debugger', { 'system.debug.terminal' } )
