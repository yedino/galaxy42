��    H      \  a   �         J   !  #   l     �  &   �  $   �     �  $   �          #  E   :     �     �  $   �  .   �  (   �  )   &  %   P     v  %   �     �  %   �  %   	  
   (	     3	     K	     f	     �	  !   �	  5   �	  
   �	     
     
  f   	
  Y   p
  %   �
  5   �
     &     >     ^     q     �     �     �     �     �     �     	          )     ;     J     _     |     �     �     �     �             -   +  %   Y  	        �     �  	   �     �     �     �     �     �     �  �    p   �  9        K  3   Z  ?   �     �    �  �   �  C  �  �   '  &   �  -     �  G  �   2  A   �  X     <   h  �  �      �   �  �   '   *"  3   R"     �"  V  �"  x   �#  <   d$  >   �$  �   �$     �%  J  �%     �'  
   �'    �'  �   �(  L   �)  N   �)  K   *  3   k*     �*  &   �*  8   �*  #   +  !   8+  �   Z+  d   �+  I   K,  4   �,  U   �,  o    -  1   �-  "   �-  e   �-  v   K.  ^   �.  U   !/     w/     �/  K   �/  )   �/  �   )0  �   �0  �   �1  3   {2  e   �2  h   3  �   ~3     4  �  14  ]   �6    67  [   K8            6       9   7                   ;          
   #       -              H   F   !             >                         =      	   ?      8   1   +   E                 &   /          <   "   :      *      @          $      %   0   B            )                G                     3      C   A              4   (   D      ,   .      2   '   5        Can not find script library $lib (dir_base_of_source=$dir_base_of_source). Configure computer for $programname Done Finished installation of $programname. How do you want to use $programname: Install L_INSTALLER_program_is_pre_pre_alpha L_fix_cgroupfs_mount L_fix_ubuntu_old_build L_fix_uninstall_apt_cacher bad=$apt_cacher_bad good=$apt_cacher_good. L_install_failed L_install_nothing_to_do L_install_option_lxcnet_bridged_INFO L_install_option_lxcnet_bridged_INPUTBOX_cards L_install_option_lxcnet_bridged_ITEM_all L_install_option_lxcnet_bridged_ITEM_some L_install_option_lxcnet_bridged_TITLE L_install_option_sudo_cap_allow L_install_option_sudo_cap_allow_title L_install_option_sudo_cap_users L_install_option_sudo_cap_users_title L_install_packages_text $packages_str L_language L_needrestart_LXC_maybe L_needrestart_summary_text L_needrestart_summary_title L_now_installing_gitian_lxc L_warning_sudo_cap_install_failed L_warning_sudo_cap_install_failed_title $packages_str L_what_now Ok Quit This seems to be a text-mode GUI, you can use up/down arrows, SPACE to select option, ENTER to finish. This tool will configure your computer for the SELECTED by you functions of $programname. We recognize your system/platform as: We will install packages: $packages_str now (as root) error_init_platforminfo error_init_platforminfo_unknown install_info_title install_packages_title install_progress_fix install_progress_title menu_main_title $programname: menu_task_autoselect menu_task_bgitian menu_task_build menu_task_devel menu_task_touse menu_task_verbose menu_task_warn menu_taskpack_custom menu_taskpack_devel_builduse menu_taskpack_normal_builduse menu_taskpack_quick_builduse menu_taskpack_quick_devel status_done_step $item status_done_step_BEFORE status_done_step_PRESSKEY tasks_we_will_do this_script_must_be_run_as_root_or $sudo_flag this_script_uses_sudo_flag $sudo_flag warn2_net warn2_net_title warn_fw warn_root warning_SUMMARY warning_SUMMARY_TITLE warning_bgitian warning_build warning_devel warning_touse $programname Project-Id-Version: galaxy 42
Report-Msgid-Bugs-To: 
POT-Creation-Date: 2017-07-17 14:31+0000
PO-Revision-Date: 2016-08-19 17:23+0000
Last-Translator:  <info@yedino.com>
Language-Team: ru
Language: ru
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=3; plural=(n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);
 Не можем найти скрипт библиотеки $lib (в директории $dir_base_of_source). Настройка компьютер для $programname Сделано Закончена установка $programname. Как Вы хотите использовать $programname: Установка Эта программа ещё не готова и не безопасна. Пожалуйста, не используйте эту версию. (Пожалуйста, используйте её только на тестовой виртуальной машине). Похоже, что эта система не поддерживает должным образом cgroupfs. Мы попытаемся исправить это. (Это необходимо чтобы использовать Gitian). Эта система (старая версия Ubuntu) требует специальные настройки для компиляции. Мы попробуем исправить это. (Это необходимо для нативной компиляции программы на этом компьютере). Мы должны удалить вашу apt cacher программу: [$apt_cacher_bad], так чтобы взамен мы установили другую нужную нам: [$apt_cacher_good]. Установка не удалась Ничего нет для установки На следующем шаге вы будете выбирать следует ли разрешить LXC использовать все сетевые адаптеры на этом компьютере или ограничить его только выбранными вручную.

ПОДСКАЗКА: Разрешить доступ ко всем сетевым адаптерам (all) более простой и удобный способ если вам пофиг.

Детали:
Gitian использует LXC для загрузки исходного кода, поэтому LXC должно получить доступ к сети. 
LXC необходим доступ только к сетевым адаптерам, которые Вы используете для доступа к Интернету(или к серверу, который имеет все необходимые файлы для сборки). 
Возможно, что у вас сейчас или в будущем будет много сетевых адаптеров в компьютере (например, WiFi, VPN, Cjdns). 
Так же возможно, что некоторые из них очень важные (например, рабочий VPN) и Вы совсем-совсем не хотите дать ниодного шанса LXC получить доступ к такому сетевому адаптеру.

Поэтому Вам необходимо выбрать (на следующем шаге)
Либо разрешить LXC использовать все и всякие сетевые адаптеры, что проще и удобнее.
Либо разрешить LXC использовать только некоторые сетевые адаптеры - Вам будет необходимо вручную сделать список этих адаптеров [eth0 eth1 wlan0...], которые LXC будет иметь право использовать. Список сетевых интерфейсов, разделённых пробелом, которые могут использовать LXC/Gitian Разрешить LXC/Gitian доступ к ВСЕМ СЕТЯМ Разрешить LXC/Gitian доступ только к некоторым сетям Gitian: LXC сеть - предоставить доступ We suggest to install and configure tool cap-tool,
so that regular user (without root) can build our program and give it special rights (like CAP_NET_ADMIN) so that without need for using root (nor sudo) he can run this program with rights to modify networking.
Users that you will sellect in a moment, will be allowed (indirectly by some program they own) to gain partiall admin privileges.
Should we use this tool? (If not, then there are other solutions, less comfortable) Sudo for cap-tool: do you agree? Select the users and/or groups that will be allowed to use our tool cap-tool to give own programs privilages CAP such as CAP_NET_ADMIN (changes to network settings).
Usually this is given to trusted users, who will compile our program here.
Provide space-separated names of users,
also you can give group name by adding character % at beginingfor example like:
%sudo %admin %developers alice Sudo for cap-tool: user/group selection Мы установик пакеты: $packages_str Language=(RU) # Возможно (но надеемся, что нет) - сеть LXC (необходима для использования Gitian) должна работать сейчас, но из-за возможных проблем Вам будет лучше перезагрузить систему, чтобы всё было хорошо. Может потребоваться перезагрузка системы из-за следующих причин: Необходима перезагрузка системы Сейчас мы устанавливаем LXC для Gitian Sudo for cap-tool: error in automatic configuration of sudo (sudoers).
You can configure this manually by using command visudo (this installer will ask you next). Sudo for cap-tool: error Для обычной компиляции программы нативно (и последующего использования) мы рекоммендуем команду:
  ./do 
И для сборки с помощью Gitian (для этой или других систем) мы рекоммендуем:
  ./build-gitian
Вы можете поменять параметры инсталятора запустив ./install.sh. Список других опций вы можете увидеть запустив ./menu или прочитав документацию. Ок Выход Это похоже на текстовый режим графического интерфейса, вы можете использовать кнопки Вверх/Вниз, Пробел для выбора опции, ENTER для завершения. Этот инструмент настроит Ваш компьютер для ВЫБРАННЫХ вами функций $programname. Мы распознали вашу систему/платформу как: Мы установим пакеты: $packages_str сейчас (как root) Нет информации об этой платформе/системе Эта платформа/ОС неизвестна Информация Пакеты для установки Автоматически решить проблему Прогресс установки Установщик $programname Пропустить некоторые вопросы в процессе установки (может быть небезопасно!) Компилировать и подписать/опубликовать (например, Gitian) Собрать эту программу из исходного кода Разрабатывать эту программу Использовать эту программу на этом компьютере (Показывать дополнительную информацию в процессе установки) Показывать предупреждения Выберите вариант... Продвинутое: собрать программу в Gitian / разрабатывать её Простой: собрать эту программу, чтобы использовать её только тут Без подтверждений (опасно!) - собрать и использовать Без подтверждений (опасно!) - для разработчиков Шаг готов: '$item'. Сейчас мы делаем: Пожалуйста, нажмите ENTER для продолжения... Мы собираемся сделать: Этот скрипт должен быть запущен с root правами или запущен с флагом $sudo_flag для использования sudo Скрипт не запущен с root правами, но вы установили флаг $sudo_flag, поэтому мы думаем что sudo готово и продолжаем работу ВНИМАНИЕ: СПЕЦИАЛЬНЫЕ настроки сетей - возможно имена или функции сетевых адаптеров (например, eth0/br0/lxcbr0) будут изменены. ВНИМАНИЕ (СПЕЦИАЛЬНЫЕ: сети) # Пожалуйста, не забудьте СЕЙЧАС установить брандмауэр! # Нам будут необходимы root-права (admin) в процессе установки. Пожалуйста, подтвердите нижеперечисленные изменения (они влияют на безопасность). Caution safety and summary * Мы собираемся настроить Ваш компьютер для использования Gitian (для виртуализации LXC). Внимание, это может создать несколько других сетевых адаптеров и изменить настройки существующих сетевых адаптеров. Для примера, IP-адрес с текущего основного Интернет адаптера (например, eth0) может быть показана/перемещена как присоединённая к другому адаптеру (вот как работает 'мост') * Установка инструментов для сборки исходного кода * Мы собираемся установить специальные инструменты разработчика, например чтобы симулировать большое количество сетей на этом компьютере (network namespace). * Настроить компьютер для работы программы $programname 