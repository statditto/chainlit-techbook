
= 章タイトル

//lead{
シンプルなウェブアプリケーションを例に、Chainlitの機能を図を多めに説明します。
//}

== 本章で利用するアプリケーション

本章では、ルールベースで応答するシンプルなウェブアプリケーション @<fn>{support} を例に、Chainlitの機能の説明を行います。アプリ構成は以下の通りで，ユーザーはブラウザからアクセスし，チャットの内容は PostgreSQL データベースに保存されます。

//footnote[support][本章のソースコード全体は以下で公開しています。@<href>{https://github.com/xxx/yyy} （※実際のURLに差し替えてください）]

//image[app_overview][アプリケーション構成図][scale=0.6]{
//}

== ログイン画面

ユーザーが Chainlit アプリケーションにアクセスすると，ログイン画面が表示されます。（@<img>{login}）

今回は、@<code>{@cl.password_auth_callback} デコレーターを修飾したコールバック関数を定義して、パスワード認証を行っています。

//image[login][ログイン画面][scale=0.6]{
//}

Chainlitはその他の認証方法もサポートしています。詳細は Chainlit のドキュメントを参照してください。

 * @<href>{https://docs.chainlit.io/authentication/overview}

認証に成功したら @<code>{cl.User} オブジェクトが返され、チャット開始画面へと遷移します。（認証に失敗したら @<code>{None} を返します。）

== チャット開始画面

//image[chat_start][チャット開始画面][scale=0.6]{
//}

ログイン後のチャット開始画面は @<img>{chat_start} のようになっています。

メッセージ入力欄以外に、アシスタント選択（画像上部）やスターター（画面中央下部）過去のチャット（画像左）などのUI要素があります。また入力欄にも歯車や猫のアイコンも存在しています。
このセクションでは、これらのUI要素の機能と実装方法について説明します。

=== スターター機能（ @<code>{@cl.set_starters} ）

=== アシスタント選択機能（ @<code>{@cl.set_chat_profiles} ）

=== チャット再開機能（ @<code>{@cl.on_chat_resume} ）

=== コマンド機能（ @<code>{cl.context.emitter.set_commands} ）

=== 入力欄の歯車アイコン（ @<code>{cl.ChatSettings} ）

== チャット画面

=== Message（ @<code>{cl.Message} ）

=== Step（ @<code>{cl.Step} ）

=== Action（ @<code>{cl.Action} ）

=== Element（ @<code>{cl.Element} ）

=== Ask User
