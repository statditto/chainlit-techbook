
= Chainlitの基本機能を概観する

//lead{
シンプルなウェブアプリケーションを例に、Chainlitの機能を図を多めに説明します。
//}

== 本章で利用するアプリケーション

本章では、ルールベースで応答するシンプルなウェブアプリケーション @<fn>{support} を例に、Chainlitの機能の説明を行います。アプリ構成は以下の通りで、ユーザーはブラウザからアクセスし、チャットの内容は PostgreSQL データベースに保存されます。

//footnote[support][本章のソースコード全体は以下で公開しています。@<href>{https://github.com/xxx/yyy} （※実際のURLに差し替えてください）]

//image[app_overview][アプリケーション構成図][scale=0.6]{
//}

== ログイン画面

ユーザーが Chainlit アプリケーションにアクセスすると、ログイン画面が表示されます。（@<img>{login}）
今回は、@<code>{@cl.password_auth_callback} デコレーターを修飾したコールバック関数を定義して、パスワード認証を行っています。

//image[login][ログイン画面][scale=0.6]{
//}

Chainlitの一部の機能は、この認証機能があることではじめて有効となります。例えば、以下の機能が挙げられます。

 * アシスタントの選択（ @<code>{@cl.set_chat_profiles} ）
 * チャット履歴の永続化（ @<code>{@cl.data_layer} ）
 * チャットの再開（ @<code>{@cl.on_chat_resume} ）

Chainlitはその他の認証方法もサポートしています。詳細は Chainlit のドキュメントを参照してください。

 * @<href>{https://docs.chainlit.io/authentication/overview}

認証に成功したら @<code>{cl.User} オブジェクトが返され、チャット開始画面へと遷移します。（認証に失敗したら @<code>{None} を返します。）

== チャット開始画面

//image[chat_start][チャット開始画面][scale=0.6]{
//}

ログイン後のチャット開始画面は @<img>{chat_start} のようになっています。
メッセージ入力欄以外に、アシスタント選択（画像上部）やスターター（画面中央下部）過去のチャット（画像左）などのUI要素があります。また入力欄直下にも歯車や猫のアイコンも存在しています。

このセクションでは、これらのUI要素の機能と実装方法について説明します。

=== スターター（ @<code>{@cl.set_starters} ）

スターターは、ユーザーがチャットを開始する際に選択できるプリセットのメッセージです。スターターをクリックすると対応するメッセージが送信され、チャットが開始されます。

スターターを設定するには、@<code>{@cl.set_starters} デコレーターを使用して、スターターのリストを定義します。個別のスターターは、@<code>{cl.Starter} クラスのインスタンスとして定義します。

//emlist[Starters][python]{
@cl.set_starters
async def set_starters() -> list[cl.Starter]:
    return [
        cl.Starter(
            label="Message",        # 表示されるラベル
            message="Hello World!", # 送信されるメッセージ
        ),
        ...
    ]
//}

スターターは、ユーザーがチャットを開始する際のガイドとして機能し、特に初めてのユーザーにとって便利です。

=== アシスタント選択（ @<code>{@cl.set_chat_profiles} ）

ChatGPTのモデル切り替えと同様の体験で、アシスタントを選択するUIを提供することが可能です。
@<fn>{note-on-chat-profile}
//footnote[note-on-chat-profile][利用には、@<hd>{ログイン画面} で述べた認証機能の有効化が必要となります。]

//image[chat_profile][Chat Profile][scale=0.6]{
//}


UIの提供は、@<code>{@cl.set_chat_profiles} と @<code>{cl.ChatProfile} を利用して実装します。

//emlist[Chat Profiles][python]{
@cl.set_chat_profile
async def set_chat_profiles() -> list[cl.ChatProfile]:
    return [
        cl.ChatProfile(
            name="Assistant Alice",
            markdown_description="A friendly assistant.",
        ),
        cl.ChatProfile(
            name="Assistant Bob",
            markdown_description="A helpful assistant.",
        ),
    ]
//}

アプリケーション側は、ユーザーが選択したアシスタントを @<code>{cl.user_session.get("chat_profile")} から取得できます。

=== チャット再開（ @<code>{@cl.on_chat_resume} ）

ChatGPTと同様に、@<img>{chat_start}の画面左には、ユーザーが過去に行ったチャット履歴が表示され、チャットを再開することができます。

この機能を利用するためには、認証機能とデータレイヤーを準備し、@<code>{@cl.on_chat_resume} に関する実装を行うという手順を行うこととなります。
後者の実装は、以下の数行で完了します。

//emlist[Chat Profiles][python]{
@cl.on_chat_resume
async def on_chat_resume(_: ThreadDict) -> None:
    pass
//}

==== データレイヤー ( @<code>{@cl.data_layer} )

チャットを再開するための当然の前提として、チャット履歴が保存されている必要があります。
Chainlitはデータレイヤーと呼ばれる抽象化層を通して、チャットデータの保存と取得を行います。
アプリケーション本体はデータベースの種類を直接意識せず、「データレイヤー」に対してデータの保存や取得を依頼するため、PostgreSQLや独自のストレージなどのさまざまな保存先を柔軟に利用することが可能です。

Chainlit はデータレイヤーの API が実装済みの @<code>{SQLAlchemyDataLayer} を提供しており、開発者はデータベースとテーブルを用意するだけで、データレイヤーとして使うことができます。
作成するテーブルは以下のリンク先から確認できます。

 * @<href>{https://docs.chainlit.io/data-layers/sqlalchemy}

特に、@<code>{users}、@<code>{threads}、@<code>{steps}というテーブルに、それぞれユーザー、チャット、チャット内のメッセージを永続化しています。@<fn>{note-on-data-layer}
//footnote[note-on-data-layer][チャットやメッセージは更新と削除が可能であることに対応して、対応するレコードもミュータブルとなっています。特に、イベントをイミュータブルに追加していく構造とはなっていません。]

//image[data_layer][Data Layerの例。 steps テーブルにメッセージが保存されている。][scale=1.0]{
//}

@<code>{SQLAlchemyDataLayer} を使う場合、アプリ側でのデータレイヤーの実装は以下の数行で済みます。

//emlist[Data Layer][python]{
@cl.data_layer
def data_layer() -> SQLAlchemyDataLayer:
    return SQLAlchemyDataLayer(conninfo=os.environ["CHAINLIT_CONNINFO"])
//}

ここで、引数 @<code>{conninfo} には、データベースの接続URIを与えています。

なお、Chainlitの @<code>{BaseDataLayer} を具象化することで、カスタムのデータレイヤーを用意することも可能です。その際に実装する API は以下のリンク先から確認できます。

 * @<href>{https://docs.chainlit.io/api-reference/data-persistence/custom-data-layer}

=== コマンド（ @<code>{cl.context.emitter.set_commands} ）

メッセージ入力欄の下にあるボタンは、コマンドと呼ばれる機能です。
ボタンを押すか、Skillsのように「/」から検索して利用することができます。
//image[command_meow_from_input][Command][scale=0.6]{
//}

コマンドを選択したあとに、ユーザーがメッセージを送信することで、対応するコマンドの処理が呼び出されます。

//image[command_wc_output][Word Count コマンドの実行結果][scale=0.6]{
//}

ユーザーが選択したコマンドは、ユーザーの送信した @<code>{cl.Message} の @<code>{command} 属性の文字列から確認でき、アプリケーション側はこの文字列をもとに処理を振り分けます。

//emlist[Command処理の実装][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    if command := message.command:
        match command:
            case "Meow":
                await cl.Message(content="Meow!").send()
            case "Word Count":
                word_count = len(message.content.split())
                await cl.Message(content=f"Word count: {word_count}").send()
        return
    ...
//}

また、コマンドを利用するためには事前にアプリケーション側にコマンドを設定しておくことが必要です。コマンドのメタ情報を @<code>{CommandDict} の辞書として記述し、そのリストを @<code>{cl.context.emitter.set_commands} で送ることで設定します。

//emlist[Commandの設定][python]{
@cl.on_chat_start
async def on_chat_start() -> None:
    ...
    await cl.context.emitter.set_commands(
        [
            {
                "id": "Meow",
                "icon": "cat",
                "description": "Sends a meow message",
                "button": True,     # ボタンを表示するかどうか
                "persistent": True, # 実行後もコマンド選択を維持するか
            },
            {
                "id": "Word Count",
                "icon": "ruler",
                "description": "Counts the number of words in the message",
                "button": True,
                "persistent": True,
            },
        ]
    )
//}

@<code>{icon} ではボタンに利用する Lucide のアイコン名を与えます。

//image[lucide][Lucide( @<href>{https://lucide.dev/} )][scale=0.8]{
//}

スターターと異なり、コマンドはユーザーからのメッセージを追加で与えることが可能です。
また、チャット開始後もコマンドは利用可能です。そのためコマンドを Skills を呼び出す入口として利用するといった使い方ができます。

=== チャット設定（ @<code>{cl.ChatSettings} ）

 @<img>{chat_start} では、コマンドボタンと並列する形で歯車のアイコンが存在しています。
これは、Chat Settings と呼ばれる機能で、チャットの細やかな設定を可能にします。
典型的には、アシスタントが利用するLLMやそのハイパーパラメータの指定を可能とします。

Chat Settings は @<code>{cl.ChatSettings} を介して設定します。
入力するウィジェットにはトグルボタンやスライダーなど様々な種類が用意されており、用途に即して使い分けることができます。

//emlist[Chat Settings][python]{
@cl.on_chat_start
async def on_chat_start() -> None:
    await cl.ChatSettings(
        [
            Select(
                id="Thinking mode",
                label="Thinking mode",
                values=["fast", "slow"],
                initial_index=0,
            ),
            Slider(
                id="Creativity",
                label="Creativity",
                initial=50,
                min=0,
                max=100,
            ),
            Switch(
                id="Enable feature X",
                label="Enable feature X",
                initial=False,
            ),
            Tags(
                id="Interests",
                label="Interests",
                values=["AI", "Machine Learning", "Data Science"],
                initial=["AI", "Data Science"],
            ),
            TextInput(id="Notes", label="Notes", placeholder="Enter your notes here"),
        ]
    ).send()
    ...
//}

ユーザーが歯車のアイコンを押すと設定画面がポップアップされます。

//image[chat_setting][チャット設定画面][scale=0.8]{
//}

設定が更新されると、@<code>{@cl.on_settings_update} に登録した関数が呼び出されます。
設定をユーザーセッションに保存することで、アプリケーション側が設定内容に応じた処理を行うことができます。

//emlist[Chat Settings Update][python]{
@cl.on_settings_update
async def setup_agent(settings: dict[str, Any]) -> None:
    cl.user_session.set("chat_settings", settings)
//}

== チャット画面

=== Message（ @<code>{cl.Message} ）

=== Step（ @<code>{cl.Step} ）

=== Action（ @<code>{cl.Action} ）

=== Element（ @<code>{cl.Element} ）

=== Ask User
