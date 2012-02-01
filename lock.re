= スレッドの排他制御

本章ではスレッドの共有リソースにアクセスするための排他制御について説明します。
GCで利用するスレッドはオブジェクトを共有リソースとして扱いますので、排他制御が必要になる場合があります。

排他制御の実装については、GCを読んでいく上で必要ありませんので詳しく説明しません。
本書では概要の説明までに留めます。

== 排他制御とは

メモリ領域を共有するスレッドでは、同じ位置にあるデータを複数のスレッドが同時に読み書きできてしまいます。
スレッドで共有しているデータが、他のスレッドから割り込みで変更される可能性があるにもかかわらず、割り込みを想定していないコードを書いてしまった場合は、思わぬところでメモリ破壊が生じ、想定不可能なエラーが発生してしまいます。

このように単一のリソースに対して、複数のスレッドから同時に処理が実行されるとまずい部分を@<b>{クリティカルセクション}と呼びます。

クリティカルセクションを扱う処理では、スレッド単体でアトミックに一連の処理を実行し、他のスレッドが割り込まないように排除する必要があります。
このように他のスレッドを排除し、あるスレッドだけでリソースを独占的に利用させることを@<b>{排他制御}と呼びます。

== ミューテックス（Mutex）

排他制御の単純な実装例としてよく利用されるのが@<b>{ミューテックス（Mutex）}です。これはmutal exclusion（相互排他）の略からきた造語です。

ミューテックスの例え話はいくつもありますが、ここでは武者さんが書かれたトイレの例@<bib>{webdb_can_thread}を取り上げてみます。

//quote{
トイレが1つしかない家に、何人かの家族が住んでいるとします。
トイレの使用にあたっては決まりがあり、ドアプレートの表示が「使用中」のときは中に入ることができず、入りたい人は外で待機します。
「空室」のプレートに当たった人は、これを裏返して「使用中」にすると中に入る権利を得て、独占的にトイレを使えます。
トイレを使い終わった人は「使用中」のプレートを「空室」に戻しますが、ほかの人がこの操作を行うことは許されません。
このとき、プレートを「使用中」に替えることをロック（lock）、「空室」に戻すことをアンロック（unlock）、トイレのことをクリティカルセクション（critical section）といいます。
//}

いくら仲のいい家族でも2人同時にトイレを利用しないですよね（きっと）。
ですので、家族ひとりひとりがスレッドだとしたら、トイレがクリティカルセクションにあたるのもうなずけます。

//quote{
このしくみにより、ドアのノックを交わす必要はなくなり、すでに人が入っているところに別の人が入ることもなくなります。
これがミューテックスです。
//}

ミューテックスは排他制御の基本的な実装であり、ミューテックスを土台として様々な排他制御が実装されます。

== モニタ（Monitor）

Javaでは言語自体に@<b>{モニタ（Monitor）}という同期機構が組み込まれています。
そして、HotspotVMの内部の排他制御はほとんどがこのモニタを使って行われます。

早速、モニタの説明に入りたいところですが一つ注意点があります。
Javaで利用されているモニタは実は一般的に知られているモニタとは少しことなります。
ですので、一般的なモニタについて知りたい場合は武者さんの記事@<bib>{webdb_can_thread}を読まれることをお勧めします。

=== Javaのモニタ

さて、Javaのモニタについてスノーボードのレンタルショップのたとえ話を持ちだしてみます。
このレンタルショップで扱うスノーボードはすべて同じサイズ・同じデザインとします。
また、店内は狭いため、一度に1人の客しか入ることができません。
もし先に客が入っていた場合は、店の前に行列を作って待ちます。
店内に客がいない場合は行列から1人店に入ることができます。店内に入った客は自分にあったスノーボードを借りて店をでます。
もし自分にあったスノーボードがなかった場合は、店に備え付けの待合室で待ちます。
スノーボードを返しに来た客も同様に行列に並びます。
返し終わった客は待合室の客の内1人、もしくは客全員を呼ぶことができます。
呼ばれていない客は待合室で呼ばれるのを待ちます。
呼ばれた客は店内に客がいないときに店内に入ります。
ただ、自分にあうスノーボードがない場合はしょうがないのでまた待合室に入って待ちます。

TODO:図

上記はモニタのたとえ話です。
この場合、共有リソースはスノーボードであり、モニタはレンタルショップを指します。
客がスレッドだとすると、スレッドはモニタの中に高々1スレッドしか入れません。
レンタルショップに客が入っている状態は、店自体にロックがかかっている状態と言えます。
客が出ると店がアンロックされ、他の客が入れるようになります。
Javaの文化では待合室で待つことを@<b>{Wait}、待合室内の1人を呼ぶことを@<b>{Notify}、全員を呼ぶことを@<b>{NotifyAll}といいます。

=== 一般的なモニタとの違い

別のたとえも少し考えてみましょう。
もし共有しているリソースがレンタルビデオだとしたらどうなるでしょうか。
来た客が対象のビデオがなかった場合に待合室に待ちます。
返却にきた客はビデオ返却後に待合室の客を呼び出しますが、呼び出された客は返されたビデオが自分の待っていたビデオとは限りません。
違った場合はまた待合室に戻ることになります。
このモニタのたとえで無駄な点は、待合室で待っている客が自分の欲しいビデオを店内に伝えられない点です。
もし「私の欲しいビデオはこれです（太郎）」と店内に張り紙できれば、返却しにきた客が張り紙を見て適切な待ち人を呼べます。
呼ばれた客は、店内に入った後で自分の欲しいビデオがなくてがっかりすることも少なくなるでしょう。
この便利な張り紙のことを@<b>{条件変数（condition variable）}と呼びます。

「Javaのモニタが一般的なモニタと異なる点」というのは実はこの部分で、一般的なモニタにはこの条件変数がありますが、Javaのモニタにはありません。

モニタが管理する共有リソースが例えばビデオのように客の要求に強く依存する場合は張り紙がある方が有利です。
返却しにきた客は待合室にいる適切な客を選ぶことが可能で、呼び出された客も関係ないときに呼ばれることが少なくなります。
Javaのモニタだと張り紙がないので、いったん待合室の全員を呼んで、呼び出された客がリソースを判断しなければなりません。

一方、モニタが管理するリソースがスノーボードの例のように客の要求に依存しないものであれば、Javaのモニタでも問題ありません。
ボード自体に個性がないため、待っている客は借りるものは何でもよく、単純に空きがでるのを待っているだけですから、張り紙は必要ないのです。

このように、Javaでは条件変数をなくしたシンプルなモニタを提供しています。

== モニタの実装概要

ここで気になるのはモニタの実装方法です。
ただ、この話題はGCの話から脱線しすぎる予感がしますので、実装の重要な部分をかいつまんで説明します。
また、ここで取り上げる実装方法はHotspotVMの例です。
モニタ実装の一例として捉えてください。

=== スレッドの一時停止・再起動

まず、行列や待合室での一時停止・再起動処理を見てみましょう。
それぞれの処理は以下のメンバ関数で実装されています。

 * @<code>{os::PlatformEvent::park()} - 待つ
 * @<code>{os::PlatformEvent::unpark()} - 再起動

parkは「駐車する」、unparkは「発車する」という意味があります。
それぞれのメンバ関数はそれぞれのOS用に実装されていますが、今回はLinuxのものを簡単に見ていきます。

@<code>{park()}では次のように@<code>{pthread_cond_wait()}を利用して待つ処理を実現しています。

//source[os/linux/vm/os_linux.cpp]{
4916: void os::PlatformEvent::park() {

4928:      int status = pthread_mutex_lock(_mutex);

4933:         status = pthread_cond_wait(_cond, _mutex);

4948: }
//}

@<code>{os::PlatformEvent}のインスタンスは@<code>{_cond}と@<code>{_mutex}のメンバ変数を保持しています。
@<code>{_cond}は条件変数、@<code>{_mutex}はミューテックスであり、それぞれPthreadsで利用される変数です。
4916行目で@<code>{pthread_mutex_lock()}を使って、@<code>{_mutex}をロックします。
その後、4933行目で@<code>{pthread_cond_wait()}を使って、現在のスレッドを一時停止状態にします。
@<code>{pthread_cond_wait()}には@<code>{_cond}と、ロック状態の@<code>{_mutex}を指定します。
@<code>{_mutex}は@<code>{pthread_cond_wait()}内部で一時停止状態になった際にアンロックされます。

@<code>{unpark()}は次のように@<code>{pthread_cond_signal()}を利用してスレッドを再起動します。

//source[os/linux/vm/os_linux.cpp]{
5011: void os::PlatformEvent::unpark() {

5028:      int status = pthread_mutex_lock(_mutex);

5034:         pthread_cond_signal (_cond);

5049: }
//}

5034行目で登場する@<code>{pthread_cond_signal()}では引数に取った条件変数で待っている1つのスレッドに対してシグナルを送り、再起動します。
ここでは@<code>{os::PlatformEvent}インスタンスの@<code>{_cond}変数で待っているスレッドに対してシグナルを送ります。

ちなみに、Windowsでは上記とほぼ同じ事を@<code>{WaitForSingleObject()}、@<code>{SetEvent()}を利用して実装しています。

@<code>{Thread}クラスは@<code>{os::PlatformEvent}クラスを継承した@<code>{ParkEvent}クラスのインスタンスをメンバ変数として保持しています。

//source[share/vm/runtime/thread.hpp]{
94: class Thread: public ThreadShadow {

       // 内部のMutex/Monitorに利用される
582:   ParkEvent * _MutexEvent ;                    

//}

そのため、@<img>{thread_park_unpark}のように、HotspotVMが管理する1スレッド（@<code>{Thread}インスタンス）の@<code>{_MutexEvent}に対して、@<code>{park()}・@<code>{unpark()}を呼ぶことで、対象のスレッドを一時停止・再起動できます。
モニタで説明した「待合室で待つ」「待合室から出る」「行列を作って待つ」などはこの@<code>{park()}・@<code>{unpark()}を利用して実装されます。

//image[thread_park_unpark][1スレッドに対してpark()を呼ぶとスレッドは一時停止する。一時停止中はCPUを無駄に利用しない。unpark()を呼ぶとスレッドは再起動する。]

=== モニタのロック・アンロック

次にモニタのロック・アンロックについて見ていきましょう。
この部分は実装が複雑なので概要だけを紹介します。

モニタの状態の一例を@<img>{monitor_lock_unlock_1}に図示しました。
このモニタでは行列（@<code>{EntryList}）にスレッドB,Cが並んで待っています。
モニタの前には小さな前室（@<code>{OnDeck}）があります。
そして、モニタ内のロックはスレッドAが現在は保持しています。

//image[monitor_lock_unlock_1][モニタの状態の一例。モニタはロックされているため、EntryListのスレッドBはロックを取ることができない。]

スレッドAのアンロック後、次の手順でスレッドBはモニタのロックを取得します。

 1. モニタをアンロックしたスレッドAが@<code>{EntryList}の先頭を@<code>{OnDeck}に格納
 2. スレッドAは@<code>{OnDeck}のスレッドBを起こす（@<code>{unpark()}）
 3. スレッドBは@<code>{OnDeck}に自分がいるのを確認
 4. スレッドBがモニタに入り、ロックする

また、スレッドAが待合室に入った場合（Wait）でも、上記と同じ手順になります。

=== スレッドのWait・Notify・NotifyAll

=== notify

== Monitorクラス

== MutexLockerクラス
