> 2014-05-17

网站配置
===========

## rails rvm安装
以root用户执行

```shell
curl -L get.rvm.io | bash -s stable
#rvm get stable #更新rvm
#如果不成功，按照出错说明去处理
#source ~/.bashrc
#source ~/.bash_profile
#sed -i -e 's/ftp\.ruby-lang\.org\/pub\/ruby/ruby\.taobao\.org\/mirrors\/ruby/g' ~/.rvm/config/db

#gpasswd -a root rvm
#gpasswd -a nginx rvm

rvm list known

rvm install 2.2.1
rvm list

rvm use 2.2.1@r4.2 --create --default
rvm gemset list

gem source -r https://rubygems.org/
gem source -a https://ruby.taobao.org

gem install rails -v='4.2' --no-rdoc --no-ri
```

## 安装passenger
以root用户执行

```shell
gem install passenger
passenger-install-nginx-module
#按照提示进行操作，可能需要安装一些包

#内存过小时可能需要制作swap
#sudo dd if=/dev/zero of=/swap bs=1M count=1024
#sudo mkswap /swap
#sudo swapon /swap

#可考虑将/opt/nginx/sbin放入PATH中
```

## 创建www用户
以root用户执行

```shell
useradd -d /var/www -s /usr/sbin/nologin www

#将相关用户加到www组
gpasswd -a root www
gpasswd -a zyx www      #平时操作的用户
gpasswd -a nobody www
```

## 创建网站目录
以普通用户执行

```shell
sudo mkdir /var/www

#配置该用户的git config与ssh key
git clone git@url:somebody/proj.git rails_app

sudo chown www:www -R /var/www/rails_app
sudo chmod g+w -R /var/www/rails_app
sudo chmod g+s -R /var/www/rails_app
sudo setfacl -R -m d:g:www:rw /var/www/rails_app

cd /var/www/rails_app
git branch production master
git checkout production

...                                       #一些配置

```

## nginx配置
修改`user`为`www`，根据需要决定是否开启access_log

```shell
vim /opt/nginx/conf/nginx.conf

user www;
worker_processes  1;

#error_log  logs/error.log;
#error_log  logs/error.log  notice;
#error_log  logs/error.log  info;

#pid        logs/nginx.pid;

events {
    worker_connections  1024;
}

http {
    passenger_root /usr/local/rvm/gems/ruby-2.2.1@r4.2/gems/passenger-5.0.11;
    passenger_ruby /usr/local/rvm/gems/ruby-2.2.1@r4.2/wrappers/ruby;

    include       mime.types;
    default_type  application/octet-stream;

    #log_format  main  '$remote_addr - $remote_user [$time_local] "$request" '
    #                  '$status $body_bytes_sent "$http_referer" '
    #                  '"$http_user_agent" "$http_x_forwarded_for"';

    #access_log  logs/access.log  main;
    sendfile        on;
    #tcp_nopush     on;
    keepalive_timeout  65;
    #gzip  on;

    server {
        listen       80;
        server_name  www.domain.cn, domain.cn;
        root /var/www/rails_app/public;
        rails_env production;
        passenger_enabled on;
        passenger_intercept_errors on;
        passenger_buffer_response off;
        client_max_body_size 500m;
        #access_log off;
    }
    server {
        ...
    }
}
```

### 使用https
使用Let's Encrypt为网站设置为https访问

#### 生成pem
> 确保网站的DNS服务器不能是 hichina dnspod cloudxns，而alidns没问题(有问题时，多试几次)

```
git clone https://github.com/letsencrypt/letsencrypt
cd letsencrypt

./letsencrypt-auto certonly --webroot -w /var/www/rails_app/public -d www.domain.cn -d domain.cn --server https://acme-v01.api.letsencrypt.org/directory
```

#### 设置nginx
设置nginx的ssl，并将普通的80端口设置跳转

```
server {
    listen 443 ssl;
    ssl_protocols TLSv1 TLSv1.1 TLSv1.2;
    ssl_certificate  /etc/letsencrypt/live/www.domain.cn/fullchain.pem;
    ssl_certificate_key  /etc/letsencrypt/live/www.domain.cn/privkey.pem;
    server_name  domain.cn,www.domain.cn;
    root /var/www/rails_app/public;
    rails_env production;
    passenger_enabled on;
    passenger_intercept_errors on;
    passenger_buffer_response off;
    client_max_body_size 500m;
    #access_log off;
}
server {
    listen      80;
    server_name    www.domain.cn,domain.cn;
    rewrite ^(.*)$  https://www.domain.cn permanent;
}
```

#### 检测

```
https://www.ssllabs.com/ssltest/analyze.html?d=domain.cn
```

根据`https://weakdh.org/sysadmin.html`文档，产生dhparams.pem

```shell
openssl dhparam -out dhparams.pem 2048
```

再次设置nginx

```
server{
    listen 443 ssl;
    ...
    ssl_ciphers 'ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:AES:CAMELLIA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA';
    ssl_dhparam /path/to/dhparams.pem;
    ssl_prefer_server_ciphers on;
}
```

重启nginx

#### 续签
Let's Encrypt要求每三个月续签一次，去做个定时任务每月一号执行

```
0 0 1 * * /var/www/letsencrypt/letsencrypt-auto --renew certonly -d www.domain.cn -d domain.cn
```

## 提示
- 生产环境中也需要git源（机器紧张时，可将git源配置在生产环境中），用于`git fetch`代码
- `Gemfile`书写时，必须将版本号写上，防止将来`bundle install`时引用了过于新的gem包
- 监控生产环境可使用`newrelic/oneapm/ganglia`等工具

## mysql数据库
```shell
apt-get install mysql-server
apt-get install mysql-client
apt-get install libmysqlclient-dev
```

shell进入mysql

```shell
mysql -u root -p
show databases;
use mysql;
show tables;
```

## 部署项目
以下的可使用普通用户执行

```shell
cd /var/www/rails_app
git checkout master
git pull origin master
git checkout production
git diff production master --
git merge master
#chmod g+w -R /var/www/rails_app

bundle install

RAILS_ENV=production rake db:setup #首次建立生产环境数据库
RAILS_ENV=production rake db:migrate

RAILS_ENV=production rake assets:precompile

sudo /opt/nginx/sbin/nginx -s reload
```

>[有关bundle](http://blog.csdn.net/huaishu/article/details/38778777)

> 有关rake db(加上`RAILS_ENV=?`执行后,则在指定环境下的数据库产生影响)
>
> 如果修改了migration，需要先rake db:rollback，然后再git merge修改后的文件，再执行rake db:migrate

```shell
rake db:charset 检索当前环境下数据库的字符设置
rake db:collation 检索当前环境下数据库的校对
rake db:create 依照目前的 RAILS_ENV 环境建立数据库
rake db:create:all 建立所有环境的数据库
rake db:drop 依照目前的 RAILS_ENV 环境删除数据库
rake db:drop:all 删除所有环境的数据库
rake db:migrate 执行Migration动作
rake db:rollback STEP=n 恢复上N个 Migration 动作
rake db:migrate:up VERSION=20080906120000 执行特定版本的Migration
rake db:migrate:down VERSION=20080906120000 恢复特定版本的Migration
rake db:version 目前数据库的Migration版本
rake db:seed 执行 db/seeds.rb 加载种子数据
```

## 分割日志
```shell
vim /etc/logrotate.conf

    /opt/nginx/logs/access.log /opt/nginx/logs/error.log /var/www/rails_app/log/production.log{
      daily
      missingok
      rotate 7
      dateext
      copytruncate
    }

logrotate -f /etc/logrotate.conf
sudo service cron restart
```

Rails Cast
===========
## [Dynamic find_by Methods](http://railscasts.com/episodes/2-dynamic-find-by-methods)
> 在rails4中被废弃了，请使用`where(...)` [ref](http://edgeguides.rubyonrails.org/4_0_release_notes.html#active-record)

```ruby
@tasks = Task.find(:all, :conditions => ['complete = ?', false])
# equals
@tasks = Task.find_all_by_complete(false)

@task = Task.find(:first, :conditions => ['complete =?', false], :order => 'created_at DESC')
# equals
@task = Task.find_by_complete(false, :order => 'created_at DESC')
```

`find_by`方法可以像`find`方法样接受`order`参数，只能使用相等比较的过滤，不能用于`like`等过滤条件。


## [Find Through Association](http://railscasts.com/episodes/3-find-through-association)
如果存在下面关系：

```ruby
class Project < ActiveRecord::Base
  has_many :tasks
end
class Task < ActiveRecord::Base
  belongs_to :project
end
```

当需要查找某项目下所有未完成任务时，可用下面表示

```ruby
@project = Project.find(params[:id])
@tasks = Task.find(:all, :conditions => ['project_id = ? AND complete = ?', @project.id, false])
```

但更好的方式是：

```ruby
@project = Project.find(params[:id])
@tasks = @project.tasks.find(:all, :conditions => [complete = ?', false])
# or (rails4 use where)
@tasks = @project.tasks.find_all_by_complete(false)
```

## [Move Find into Model](http://railscasts.com/episodes/4-move-find-into-model)
如果多处用到的查询，可以将其封装到模型中：

```ruby
class Task < ActiveRecord::Base
  belongs_to :project

  def self.find_incomplete
    find_all_by_complete(:false, :order => "created_at DESC")
  end
end

# use
@tasks = Task.find_incomplete
```

别忘了在方法名前添加`.self`，命令中不需要指明调用哪个`Task`了，因为查找的作用域在类内部。增加了方法后，我们就可以调用封装后的`Task.find_incomplete`方法代替之前的写法。甚至在 __级联查询__ 的时候依然生效，比如查询在`Project`下的所有未完成任务（上一个例子）：

```ruby
@project = Project.find(params[:id])
@tasks = @project.tasks.find_incomplete
```

## [Using with_scope](http://railscasts.com/episodes/5-using-with-scope)
上例中有一个限制，无法再向方法中传递参数，如`:limit=>20`，我们需要让`Task`中的`find_incomplete`接受一个哈希参数；然后再方法实现中将传入的参数合并后传入`find`方法来实现这个功能。当然了，有更优雅的实现方式那就是使用`find_scope`方法向`find`方法传递参数。

```ruby
class Task < ActiveRecord::Base
  belongs_to :project

  def self.find_incomplete(options = {})
    with_scope :find => options do
      find_all_by_complete(false, :order => 'created_at DESC')
    end
  end
end
# use
@tasks = @project.tasks.find_incomplete :limit => 20
```

当`find`方法执行时会连同`with_scope`中指定的条件。这样一来`find_incomplete`方法就能携带任何传入的过滤条件参数了。这一特性在其他的作用域内也能生效。下面的代码中，我们传入了限制结果数量的参数。`find`方法传入了 __两层作用域__：第一层是得到指定项目中的所有未完成任务，第二层是结果限制返回前20个。

## [All About Layouts](http://railscasts.com/episodes/7-all-about-layouts)
- __全局范围布局文件__：布局文件的标准存放位置在`/app/view/layouts`目录。名为`application.html.erb`的布局规则将会作用域所有的控制器和页面请求中。
- __特定于控制器的布局文件__：通过把控制器的名称指定给布局文件，来将布局文件和控制器关联。这样一来，这个布局文件就会作用到所有指向这个控制器页面。比如说，在`layouts`目录建立一个`projects.html.erb`文件意味着这个布局只在`Project`对应的控制器生效(全局的将失效)。
- __几个控制器想共享一个布局文件__：比如说，我想共享名字叫admin的那个布局文件。那就在控制器代码中明确指定吧：`class ProjectsController < ApplicationController; layout "admin"; end`。在控制器中通过layout方法指定的布局文件会覆盖通过文件名字规则指定的布局定义。
- __动态计算关联布局文件__：比如说我们只是想让用户登录之后再让页面关联名为`admin`的布局文件。可以通过给`layout`方法传递参数的方式实现；定义一个与参数名相同的方法，方法 __逻辑中计算__ 在什么时候应该使用哪个名字的布局文件即可。

```ruby
class ProjectsController < ApplicationController
  layout :user_layout

  def index
    @projects = Project.find(:all)
  end

  protected
  def user_layout
    if current_user.admin?
      "admin"
    else
      "application"
    end
  end
end
```
- __动态渲染关联布局文件__：通过`render`方法，可以更加精细地设定哪个控制器的 __哪个请求__ 使用哪个布局文件。

```ruby
render :layout => false
```

## [Layouts and content_for](http://railscasts.com/episodes/8-layouts-and-content-for)
如果需要对当前关联的布局文件进行更加细节上的调整，比如说，想在`index`页面上使用不同于全局页面布局文件的`CSS`定义等。在页面中使用`content_for`方法可以实现想要的效果。`content_for`接受一个名称参数及代码块定义。代码块逻辑生成的内容将会被加载到布局文件中。

```ruby
<% content_for :head do %>
  <%= stylesheet_link_tag 'projects' %>
<% end %>
<h2>Projects</h2>
<ul>
<% for project in @projects %>
  <li><%= project.name %></li>
<% end %>
</ul>
```

此时的布局文件如下：

```ruby
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
  <head>
    <title>Todo List</title>
    <%= stylesheet_link_tag 'application' %>
    <%= yield :head %>
  </head>
  <body>
    <div id="container">
      <h1>Todo List</h1>
      <%= yield %>
    </div>
  </body>
</html>
```

可理解为在布局文件中留有占位符供使用方来定制

## [Filtering Sensitive Logs](http://railscasts.com/episodes/9-filtering-sensitive-logs)
在`ApplicationController`中增加了`config.filter_parameters += [:password]`（application.rb中）。可以通过名字指定对输出到日志的内容进行保护。日志中`password`的值被`[FILTERED]`遮盖住了，不再以明文显示。

虽然能在SQL语句中看到明文，但请放心的是，这只是开发模式下才会有的内容，发布模式（product）下不会记录SQL语句。再说，本来也不应该向数据库中存放明文密码，而是应该加密混淆后在入库。

## [Performing Calculations on Models](http://railscasts.com/episodes/14-performing-calculations-on-models)
`ActiveRecord`预制了好多对模型中数据进行统计计算的方法。

```ruby
Task.sum(:priority)
# the SQL generated
# SELECT sum("tasks".priority) AS sum_priority FROM "tasks";
Task.sum(:priority, :conditions => {:completed => false })
# SELECT sum("tasks".priority) AS sum_priority FROM "tasks" WHERE (completed="f");
```

这些统计方法都是 __类方法__，可以在 __级联查询__ 中使用。比如`Project`具有`has_many :tasks`

```ruby
project = Project.first
project.tasks.sum(:priority, :conditions => {:completed => true})
# SELECT sum("tasks".priority) AS sum_priority FROM "tasks" WHERE ("tasks"."completed" = 't') AND ("tasks".project_id = 1);
```


http://railscasts.com/episodes/15-fun-with-find-conditions




```ruby

```



<!-- ImageMagick -->
