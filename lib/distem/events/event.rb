require 'distem'


module Distem
  module Events

    class Event

      # Type of resource : vnode, vcpu, viface
      attr_reader :resource_type

      # Type of change to apply : churn, power, bandwith, latency
      attr_reader :change_type

      # Vnode associate to this resource
      attr_reader :vnode_name

      # VIface name, if the resource is a VIface
      attr_reader :viface_name

      # VIface direction, if the change is on bandwith or latency ('out' or 'in')
      attr_reader :viface_direction

      # Event value : what will be done on the resource - churn : 'up' or 'down', a number otherwise
      attr_reader :event_value

      def initialize(resource_type, change_type, event_value, vnode_name, viface_name = nil)
        raise if resource_type=='viface' and not viface_name)
        raise if change_type == 'power' and resource_type != 'vcpu'
        raise if (change_type == 'bandwith' or  change_type == 'latency') and resource_type != 'viface'
        raise if (change_type == 'churn' and resource_type == 'vcpu')
        raise if (change_type == 'churn' and :event_value != 'up' and event_value != 'down')
        raise if (change_type == 'bandwith' or  change_type == 'latency') and :viface_direction != 'up' and :viface_direction != 'down'

        @resource_type = resource_type
        @change_type = change_type
        @vnode_name = vnode_name
        @viface_name= viface_name
        @event_value = event_value
      end

      def trigger(event_list = nil)

        cl = NetAPI::Client.new
        if @change_type == 'churn'
          if @resource_type == 'vnode'
            if @event_value == 'up'
              cl.vnode_start(@vnode_name)
            else
              cl.vnode_shutdown(@vnode_name)
            end
          else
            raise "Not implemented : #{@change_type} on #{@resource_type}"
          end
        elsif @change_type == 'power'

        elsif @change_type == 'bandwith'

        elsif @change_type == 'latency'

        else
          raise "Not implemented : #{@change_type}"
        end
      end

    end

  end
end
