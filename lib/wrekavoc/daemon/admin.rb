require 'net/ssh'
require 'wrekavoc/resource/pnode'

module Wrekavoc

  module Daemon

    class Admin
      PATH_WREKANETAPI='~/'
      PATH_BIN_RUBY='/usr/bin/ruby'
      PATH_SSH_KEY='/root/.ssh/id_rsa'

      def initialize
      end

      def pnode_run_server(pnode)
        raise unless pnode.is_a?(Resource::PNode)

        if pnode.status == Resource::PNode::STATUS_INIT
          Net::SSH.start(pnode.address, pnode.ssh_user, :keys => PATH_SSH_KEY) do |ssh|
            ssh.exec!("#{PATH_BIN_RUBY} -rubygems #{PATH_WREKANETAPI}/server.rb " \
                      "1>/dev/null &>/dev/null &")
          end
          pnode.status = Resource::PNode::STATUS_RUN
        end
      end
    end

  end

end